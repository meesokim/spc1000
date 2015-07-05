/*
	PS/2 キーボードを SHARP X1 につなぐ
	PS/2 キーボードの受信処理
	
	クロックの立ち下がりで1ビットづつ読むだけなので、簡単です。
	スタートビット 1
	データビット   8
	パリティビット 1
	ストップビット 1
	
	計11ビット 奇数パリティ
	送られてくるキーコードは http://kyoutan.jpn.org/uts/pc/pic/x68key/ に書きました。



	2014年7月22日 作成
	
	佐藤恭一  http://kyoutan.jpn.org/

	無保証です。
	佐藤恭一が作成した部分は用途に制限を設けません。商用・非商用にかかわらず自由に使用して頂いて構いません。
	勝手に複製したり、改造したり、配布したり、売ったりしても良いということです。
	連絡不要です。
*/

#include "sfr_r8m12a.h"
#include "ps2.h"
#include "iodefine.h"

volatile unsigned short PS2TIMER=0;		// PS2受信タイムアウトタイマー
volatile unsigned char PS2BUFF[PS2BUFFSIZE];	// PS2受信バッファ
volatile unsigned char PS2RPOS=0;		// PS2読み出し位置
volatile unsigned char PS2WPOS=0;		// PS2書き込み位置

void ps2key_init(void)
{
	/* INT0 外部割込み初期化 */
	//INT0 PS/2 CLOCK
	intf0=0b00000001;	// INT0 f1フィルタ使用 1*3/18.432=0.16us
	iscr0=0b00000000;	// INT0 立ち下がりエッジ
	inten=0b00000001;	// INT0 入力許可
	{
		unsigned char a;
		for(a=(6*8); a!=0; a--) asm("nop");		// ちょっと時間待ち
	}
	// PMLi PMHi ISCR0 INTEN KIEN レジスタを書き換えると割り込み要求フラグが 1になることがある
	// とマニュアルに書いてあるのでフラグをクリアする
	while(1==iri0) iri0=0;
}

// 外部割込み INT0
// PS/2 CLOCK の立ち下がりで割り込みをかけて、1ビットづつデータを取り込む
#pragma INTERRUPT INT_int0 (vect=29)
void INT_int0(void)
{
	static unsigned short bit=1;
	static unsigned short data=0;
	static unsigned char parity=0;
	
	// 受信動作中で止まっていたら状態クリアして最初から受信
	if((bit != 1) && (PS2TIMEOUT < PS2TIMER))
	{
		bit=1;
		data=0;
		parity=0;
	}
	
	// 下位ビットから1ビットづつ受信
	if(0!=PS2DATA)
	{
		// 1
		data+=bit;
		parity++;
	}
	
	if(0b100_0000_0000==bit)	// 11bit 読んだ （スタートビット1 データビット8 パリティ1 ストップ1）
	{
		parity--; // ストップビット分を引く
		if(0!=(parity & 1))	// パリティチェック 1が奇数なら正常
		{
			// 正常受信
			if((PS2BUFFSIZE-1) > ps2size())//バッファに空きがあるか？
			{
				PS2BUFF[PS2WPOS]=((data >> 1) & 0xFF);
				
				if((PS2BUFFSIZE-1) > PS2WPOS)
				{
					PS2WPOS++;
				}
				else
				{
					PS2WPOS=0;
				}
			}
			else
			{
				// バッファフル
			}
		}
		else
		{
			// パリティエラー
		}

		bit=1;
		data=0;
		parity=0;
	}
	else
	{
		if((1==bit)&&(data!=0))	
		{	//スタートビットがゼロじゃない 状態リセット
			bit=1;
			data=0;
			parity=0;
		}
		else
		{	// 次のビットを読む準備
			bit=(bit<<1);
			PS2TIMER=0;	// タイムアウトタイマークリア
		}
	}
	
	while(1==iri0) iri0=0;	// 自動的に割り込みフラグクリアされるので、この行無くていい
}

// バッファの有効データ数を返す
unsigned char ps2size(void)
{
	signed int size;
	
	size=(signed int)PS2WPOS-PS2RPOS;
	if(0>size)
	{
		size=PS2BUFFSIZE+size;
	}
	
	return size;
	// size=5 wpos=2 rpos=3 4
}

// 受信バッファをクリアする
void ps2clear(void)
{
	PS2WPOS=0;
	PS2RPOS=0;
	PS2BUFF[PS2RPOS]=0;
}

// バッファから1byte読む
unsigned char ps2read(void)
{
	unsigned char data=0;

	if(PS2WPOS!=PS2RPOS)	// バッファにデータはあるかな？
	{
		data=PS2BUFF[PS2RPOS];

		if((PS2BUFFSIZE-1) > PS2RPOS)
		{
			PS2RPOS++;
		}
		else
		{
			PS2RPOS=0;
		}
	}
	return data;
}

// 受信するまで待って1byte読む
unsigned char ps2get(void)
{
	while(0==ps2size());	//バッファにデータが入るまで待つ
	return ps2read();
}
