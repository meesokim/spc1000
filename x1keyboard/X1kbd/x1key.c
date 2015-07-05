/*
	PS/2 キーボードを SHARP X1 につなぐ
	X1 キーボードの送信処理
	
	X1センターさん (http://www.x1center.org/) の、キーボードの伝送フォーマット仕様を参考にしました。
	X1キーボードの実物を持っていないので、大変助かりました。
	X1センターさんの資料は「使える物があれば自由に使って下さい」とのことです。
	

	2014年7月22日 作成
	
	佐藤恭一  http://kyoutan.jpn.org/

	無保証です。
	佐藤恭一が作成した部分は用途に制限を設けません。商用・非商用にかかわらず自由に使用して頂いて構いません。
	勝手に複製したり、改造したり、配布したり、売ったりしても良いということです。
	連絡不要です。
*/


#include "sfr_r8m12a.h"
#include "x1key.h"
#include "iodefine.h"

unsigned short SEND_DATA;	// X1送信データ

void X1_send(unsigned short data)
{
	while(0 != tcstf_trjcr);	// 送信中なら終わるまで待つ
	
	SEND_DATA=data;

	/* TIMER RJ */
	trjioc=	0b00000011;	/* TRJO Hから出力開始、TRJIO トグル出力無し */
	trjmr=	0b00000001;	/* パルス出力モード、分周無し */
	trjcr=	0b00000100;	/* カウント停止 出力ピン初期化 */
	pmh3 =	0b10000000;	// P3_7 TRJO X1KEYOUT を TRJO に 
	trjie_trjir=1;	/* TIMER RJ 割り込み許可 */
	trj=TRJ250us;	// 250us セット （250us 後に L ヘッダー分）250usに意味は無い。もっと短くても良い
	tstart_trjcr=1;	// タイマースタート
}

void x1key_init(void)
{
	X1KEYOUT=1;
	
	/* TIMER RJ 初期化*/
	msttrj=0;	// スタンバイ解除
	trjcr=	0b00000100;	/* カウント停止 */
	trjioc=	0b00000011;	/* TRJO Hから出力開始、TRJIO トグル出力無し */
	trjmr=	0b00000001;	/* パルス出力モード、分周無し */
	trjcr=	0b00000100;	/* カウント停止 出力ピン初期化 */
}

// TIMER RJ アンダーフロー割り込み
// タイマーの機能で、ダウンカウンタがアンダーフローするたびに、
// 出力ピン (TRJO) の出力が反転します
#pragma INTERRUPT INT_trj (vect=22)
void INT_trj(void)
{
	static unsigned char count=0;
	
	if(0==(count&1))	// 偶数 L 期間
	{	// 0 2 4 6 8 10 12 14 16 18 20 22 24 26 28 30 32 34 36
		switch(count)
		{
			case 0:		// header L
				trj=TRJ1000us;	// 1000us
				break;
				
			case 36:	// stop 終了
				while(1==tcstf_trjcr) tstart_trjcr=0; // タイマー停止（ビット操作だと出力初期化されないよう）
				trjcr=	0b00000100;	// カウント停止 出力ピン初期化
				pmh3 =0b10000000;	// P3_7 TRJO X1KEYOUT を TRJO に 
				break;
			
			default:
				trj=TRJ250us;	// 250us
				break;
		}
	}
	else				// 奇数 H 期間
	{	// 1 3 5 7 9 11 13 15 17 19 21 23 25 27 29 31 33 35
		switch(count)
		{
			case 1:		// header H
				trj=TRJ700us;
				break;
					
			//case 3:		// start H	スタートビットいらない
			//trj=TRJ750us;
			//	break;
					
			case 35:	// stop H 期間
				X1KEYOUT=1;	// H 出力
				pmh3 =0b00000000;	// P3_7 TRJO X1KEYOUT をI/Oポートに 反転出力動作終了
				trj=TRJ1750us;
				break;
					
			default:	// 上位ビットから16ビット送信
				if(0==(SEND_DATA & 0x8000))
				{
					trj=TRJ750us;	// 0
				}
				else
				{
					trj=TRJ1750us;	// 1
				}
				SEND_DATA=(SEND_DATA<<1);
				break;
		}
	}
	
	count++;
	if(36<count) count=0;
	
	while(1==trjif_trjir) trjif_trjir=0;	// 割り込みフラグクリア
}

// デバッグ用 1バイトを16進2桁で送出（表示）
void puth2(unsigned char a)
{
	X1_send(((unsigned short)0b10111111 << 8) + tochar((a&0xF0)>>4));//押す
	X1_send(((unsigned short)0b11111111 << 8) + 0x00);//離す
	
	X1_send(((unsigned short)0b10111111 << 8) + tochar(a&0x0F));//押す
	X1_send(((unsigned short)0b11111111 << 8) + 0x00);//離す

	X1_send(((unsigned short)0b10111111 << 8) + 0x20);//押す
	X1_send(((unsigned short)0b11111111 << 8) + 0x00);//離す
}

// デバッグ用 0～15の数を文字に変換
unsigned char tochar(unsigned char a)
{
	if(a<10) a=a+0x30;
	else a=a+0x41-10;
	return a;
}
