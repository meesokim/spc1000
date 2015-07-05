/*
	PS/2 キーボードを SHARP X1 につなぐ
	100[ms] のインターバルタイマー
	
	PS/2 の受信タイムアウトで使っています。
	

	2014年7月22日 作成
	
	佐藤恭一  http://kyoutan.jpn.org/

	無保証です。
	佐藤恭一が作成した部分は用途に制限を設けません。商用・非商用にかかわらず自由に使用して頂いて構いません。
	勝手に複製したり、改造したり、配布したり、売ったりしても良いということです。
	連絡不要です。
*/

#include "sfr_r8m12a.h"
#include "timer.h"
#include "ps2.h"
#include "iodefine.h"

volatile unsigned short TIMER=0;		// 100[ms]タイマー

void timer_init(void)
{
	// TIMER RB2 初期化 100ms インターバルタイマー
	msttrb=0;	// スタンバイ解除
	trbcr=0b00000100;	// カウント停止
	trbmr=0b01100100;	// f64分周 16ビット タイマモード
/*
 100[ms] = 100000[us]	=  1843200
 						f2  921600
						f4  460800
						f8  230400
						f16 115200
						f32  57600
						f64  28800 = 0x7080
*/
	// 100[ms] = f64 0x7080
	trbpre=0x80;	// 下位8ビット
	trbpr=0x70;		// 上位8ビット
	trbie_trbir=1;	// 割り込み許可
}

/*
void timer_start(void)
{
	tstart_trbcr=1;	// TIMER RB2 カウント開始 
}
*/

#pragma INTERRUPT INT_trb (vect=24)
void INT_trb(void)
{
	TIMER++;
	if(0xFF != PS2TIMER) PS2TIMER++;	// タイムアウト処理用オーバーフローさせない
	
	while(trbif_trbir==1) trbif_trbir=0;	// 割り込みフラグクリア
}
