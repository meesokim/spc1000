/*
	PS/2 キーボードを SHARP X1 につなぐ
	キーコード変換処理

	2014年7月23日作成
	
	佐藤恭一  http://kyoutan.jpn.org/

	無保証です。
	佐藤恭一が作成した部分は用途に制限を設けません。商用・非商用にかかわらず自由に使用して頂いて構いません。
	勝手に複製したり、改造したり、配布したり、売ったりしても良いということです。
	連絡不要です。
*/

#include "keyconv.h"
#include "keytable.h"
#include "ps2.h"
#include "x1key.h"

volatile unsigned short x1shift=0xFF;	// X1 シフト状態保存 0で有効
#define TENKEY	((unsigned char)(1<<7))
#define PRESS	((unsigned char)(1<<6))
#define REPEAT	((unsigned char)(1<<5))
#define GRAPH	((unsigned char)(1<<4))
#define CAPS	((unsigned char)(1<<3))
#define KANA	((unsigned char)(1<<2))
#define SHIFT	((unsigned char)(1<<1))
#define CTRL	((unsigned char)(1<<0))


volatile unsigned char ps2ex=0;			// PS2キーボード 拡張キーフラグ
#define EXKEY		((unsigned char)(1<<0))
#define RELEASE		((unsigned char)(1<<1))
#define PAUSE_BREAK	((unsigned char)(1<<2))


unsigned char codeconv(unsigned char data);
unsigned char checkbreak(void);
unsigned char x1code(unsigned char data);
void x1trans(unsigned char data);


void keyconv(void)
{
	unsigned char data;
	
	data=ps2get();	// PS/2キーボードから受信するまで待って、1バイト読み込む
	switch(data)
	{
		case 0xE0:	// 拡張キー
			ps2ex|=EXKEY;	// 拡張キーフラグセット
			break;
		
		case 0xF0:	// 離した
			ps2ex|=RELEASE;	// 離したフラグセット
			break;
			
		default:
			// PS/2 のコードから内部コードに変換
			if(0xE1==data) data=checkbreak();	// PAUSE/BREAK キーの判別
			
			if(0==(ps2ex & EXKEY))
			{	// 通常キー
				if(0x8F<data) data=0x00;
				data=KEY106[data];
			}
			else
			{	// 拡張キー
				if(0x5F<data) data=KEY106[data + 0x30];
				else data=KEY106[data];
			}
			ps2ex&=~EXKEY;	// 拡張キーフラグリセット
			
			x1trans(x1code(data));	// 内部コードからX1のコードに変換して送信
			break;
	}
}

// 内部コードからX1のコードに変換
unsigned char x1code(unsigned char data)
{
	x1shift|=TENKEY;	// 特殊キーフラグリセット
	// テンキーやファンクションキー SHIFT CTRL なんかの特殊キーかチェック
	if(((0x3A<data)&&(0x59>data))||(0x5A<data)) x1shift&=~TENKEY;	// 特殊キーフラグセット
			
	switch(data)	// 内部コードで分岐
	{
		case GRAPH_CODE1:
		case GRAPH_CODE2:
			if(0==(ps2ex&RELEASE)) x1shift&=~GRAPH;	// 押した
			else x1shift|=GRAPH;	// 離した
			data=0;
			break;

		case CAPS_CODE:
			if(0!=(ps2ex&RELEASE)) x1shift=(x1shift|CAPS)&(~(x1shift&CAPS));	// 離すたびトグル
			data=0;
			break;

		case KANA_CODE:
			if(0!=(ps2ex&RELEASE)) x1shift=(x1shift|KANA)&(~(x1shift&KANA));	// 離すたびトグル
			data=0;
			break;
					
		case SHIFTL_CODE:
		case SHIFTR_CODE:
			if(0==(ps2ex&RELEASE)) x1shift&=~SHIFT;	// 押した
			else x1shift|=SHIFT;	// 離した
			data=0;
			break;
				
		case CTRL_CODE:
			if(0==(ps2ex&RELEASE)) x1shift&=~CTRL;	// 押した
			else x1shift|=CTRL;	// 離した
			data=0;
			break;

		case UP_CODE:
			data=0x1E;
			break;
				
		case DOWN_CODE:
			data=0x1F;
			break;
				
		case LEFT_CODE:
			data=0x1D;
			break;
				
		case RIGHT_CODE:
			data=0x1C;
			break;

		case INS_CODE:
			data=0x12;
			break;

		case DEL_CODE:	// BSは通常キーのテーブルに入っている
			data=0x08;
			break;

		case HOME_CODE:
			x1shift&=~SHIFT;	// +SHIFT
			data=0x0B;
			break;
					
		default:	// 通常キー 内部コードからX1のコードに変換
			data=codeconv(data);
			break;
	}
	
	return data;
}

// PAUSE/BREAK キーは通常と違うコードで始まるので、別処理で判別
unsigned char checkbreak(void)
{
	unsigned char a;
	for(a=1; a<8; a++)
	{
		if(ps2get()!=BREAK_CODE[a]) break;
	}
	
	if(8==a) // BREAKキー だった
	{
		ps2ex|=PAUSE_BREAK;
		return 0x80;
	}
	
	return 0x00; // BREAKキー じゃなかった
}

// X1に送信
void x1trans(unsigned char data)
{
	static unsigned char lastshift=0xFF;

	if(0==(ps2ex&RELEASE))
	{	// 押した
		x1shift&=~PRESS;
			
		// ゼロじゃ無い時とシフト状態変化した時送信
		if((0x00!=data)||(x1shift!=lastshift)) X1_send(((unsigned short)x1shift << 8) | data);
		
		if(0!=(ps2ex&PAUSE_BREAK))	// PAUSE/BREAKの時リリースコードも送る
		{
			x1shift|=TENKEY;	// 特殊キーフラグクリア
			X1_send(((unsigned short)x1shift << 8) | 0x00);
			ps2ex&=~PAUSE_BREAK;
		}
	}
	else
	{	// 離した
		x1shift|=PRESS;
		X1_send(((unsigned short)x1shift << 8) | 0x00);
		ps2ex&=~RELEASE;	// 離したフラグクリア
		if(0x0B==data) x1shift|=SHIFT;		// HOMEの時 -SHIFT
	}

	lastshift=x1shift;
}

unsigned char codeconv(unsigned char data)
{
	unsigned char status;

	status=(x1shift & (CAPS | GRAPH | KANA | SHIFT | CTRL));
	
	if((KANA | GRAPH | SHIFT | CTRL )==status) 						// CAPS
			{
				unsigned char ret;
				ret=CHR_TBL0[data];
				if((0x60 < ret)&&(0x7b > ret)) ret-=0x20;	// アルファベット大文字に
				return ret;
			}
	if((KANA | GRAPH | CTRL         )==status)							// CAPS+SHIFT
			{
				unsigned char ret;
				ret=CHR_TBL1[data];
				if((0x40 < ret)&&(0x5b > ret)) ret+=0x20;	// アルファベット小文字に
				return ret;
			}
			
	status&=(GRAPH | KANA | SHIFT | CTRL);
	
	if((GRAPH | KANA  | CTRL )==status) return CHR_TBL1[data];	// SHIFT
	if((GRAPH | KANA  | SHIFT)==status) return CHR_TBL3[data];	// CTRL
	if((KANA  | SHIFT | CTRL )==status) return CHR_TBL2[data];	// GRAPH
	if((        SHIFT | CTRL )==status) return CHR_TBL2[data];	// GRAPH+KANA
	if((GRAPH | SHIFT | CTRL )==status) return CHR_TBL4[data];	// KANA
	if((GRAPH | CTRL         )==status) return CHR_TBL5[data];	// KANA+SHIFT

	return CHR_TBL0[data];	// シフトなし
}