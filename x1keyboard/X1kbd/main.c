/*
	PS/2 キーボードを SHARP X1 につなぐ
	
	Renesas R8C/M12A 用
	標準の プログラムROM:2KB データフラッシュ:2KB RAM:256B に収まります。

	2014年7月16日 作成

		
	佐藤恭一  http://kyoutan.jpn.org/
	
	無保証です。
	X1のキーボード実物を所有していないので、動作が正しいのかよくわかりません。
	動作テストは X1 turbo III で行いましたが、概ねいい感じに動いているようです。
	
	佐藤恭一が作成した部分は用途に制限を設けません。商用・非商用にかかわらず自由に使用して頂いて構いません。
	勝手に複製したり、改造したり、配布したり、売ったりしても良いということです。
	連絡不要です。
*/

#include "sfr_r8m12a.h"
#include "iodefine.h"
#include "x1key.h"
#include "ps2.h"
#include "timer.h"
#include "keyconv.h"

void main(void);
void osc_init(void);
void io_init(void);

void main(void)
{
	DI();	// 割り込み禁止
	osc_init();
	io_init();
	timer_start();
	EI();	// 割り込み許可
	
/*	
	// PS/2 リードテスト
	while(TRUE)
	{
		puth2(ps2get());
		//puth2(PS2RPOS);
		//puth2(PS2WPOS);
	}
*/

/*
	// X1 送出テスト
	{
		unsigned short a=0;
		unsigned char b=0x20;

		while(TRUE)
		{
			if(10 < (TIMER-a))	// 1秒毎
			{
				a=TIMER;
			
				X1_send(((unsigned short)0b10111111 << 8) + b);//押す
				X1_send(((unsigned short)0b11111111 << 8) + 0x00);//離す
			
				if(0x7F==b)
				{
					b=0x20;
				}
				else
				{
					b++;
				}
			}
		}
	}
*/

	while(TRUE)
	{
		keyconv();
	}
}

void osc_init(void)
{
	// 内蔵高速オシレーターに切り替え
	prc0=1;				// クロックレジスタアクセス許可
	ococr=0b00000001;	// 高速オンチップオシレーター発振 低速も発振
	{
		unsigned char a;
		for(a=0; a<255; a++)  asm("nop");	// ここでオシレーターの発振が安定するのを待てとあるので適当に時間待ち
	}
	sckcr=0b01000000;	// XIN/高速オシレーター選択で高速を選択 CPUクロック分周無し
	ckstpr=0b10000000;	// システムクロック低速/高速選択で高速を選択
	phisel=0x00;		// システムクロック分周無し
	frv1=fr18s0;		// 高速オンチップオシレーターを18.432MHzに調整
	frv2=fr18s1;
	prc0=0;				// クロックレジスタアクセス禁止
}

void io_init(void)
{
	// I/O ポート
	// P1_0 
	// P1_1 
	// P1_2 
	// P1_3
	// P1_4 TXD 書き込み・通信用
	// P1_5 RXD 通信用		(TRJIO)
	// P1_6 RXD 書き込み用	(TRJO)
	// P1_7

	// P3_3 PS/2 DATA
	// P3_4
	// P3_5
	// P3_7 TRJO X1KEYOUT

	// P4_2
	// P4_5 INT0 PS/2 CLK
	// P4_6 (TRJIO X1KEYOUT TRJOの反転出力 使わない)
	// P4_7

	// PA_0
	
	X1KEYOUT=1;
	p1_4=1;	// TXD

	// ポートの向き 1:出力
	pd1=0b10011111;	// P1_5 P1_6 入力 RXD
	pd3=0b11110111;	// P3_7 TRJO X1KEYOUT 出力 | P3_3 PS/2 DATA 入力
	pd4=0b11011111;	// P4_5 INT0 PS/2 CLK 入力
	
	// プルアップ 1:あり
	pur1=0b01100000;
	pur3=0b00001000;
	pur4=0b00100000;
	
	// オープンドレイン出力 1:あり
	pod1=0b00000000;
	pod3=0b00001000;
	pod4=0b00100000;

	x1key_init();
	ps2key_init();	
	timer_init();
		
	// 割り込み優先レベル
	ilvlb=0x01;		// TIMER RJ	1
	ilvlc=0x01;		// TIMER RB 1
	ilvle=0x20;		// INT0		2	TIMER RJ より優先度高い
	// 割り込み応答に 20サイクル超と結構かかる。
	
	asm("LDIPL #0");	// プロセッサ割り込み優先レベル0（この値よりも高いレベルの割り込みが受け付けられる）

	// ポートマッピング
	pml1 =0b00000000;
	pmh1 =0b00000101;	// P1_4:TXD P1_5:RXD
	pmh1e=0b00000000;
	pml3 =0b00000000;
	pmh3 =0b10000000;	// P3_7 TRJO X1KEYOUT
	pml4 =0b00000000;
	pmh4 =0b00000100;	// P4_5 INT0 PS/2 CLK
	pmh4e=0b00010000;
	pamcr=0b00010001;	// PAはリセット
}
