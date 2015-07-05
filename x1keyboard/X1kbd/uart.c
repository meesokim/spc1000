 
#include "sfr_r8m12a.h"
#include "uart.h"

// バッファサイズは2の倍数じゃなきゃダメ
// バッファサイズを変更する場合スタックサイズを調整してRAMが溢れないように
/*#define	buffsize 32

volatile unsigned char buff[buffsize];
volatile unsigned char readpos,writepos;
*/
void osc_init(void)
{
	// 内蔵高速オシレーターに切り替え
	prc0=1;				// クロックレジスタアクセス許可
	ococr=0b00000001;	// 高速オンチップオシレーター発振 低速も発振
	{
		unsigned char a;
		for(a=0; a<255; a++);	// ここでオシレーターの発振が安定するのを待てとあるので適当に時間待ち
	}
	sckcr=0b01000000;	// XIN/高速オシレーター選択で高速を選択 CPUクロック分周無し
	ckstpr=0b10000000;	// システムクロック低速/高速選択で高速を選択
	phisel=0x00;		// システムクロック分周無し
	frv1=fr18s0;		// 高速オンチップオシレーターを18.432MHzに調整
	frv2=fr18s1;
	prc0=0;				// クロックレジスタアクセス禁止
}	

void uart_init(void)
{
	// UART0の設定
	p14sel0=1;
	p14sel1=0;
	p14sel2=0;	// P1_4をTXD
	
	p15sel0=1;
	p15sel1=0;
	p15sel2=0;	// P1_5をRXD

//#define RTS p1_0
//	pd1_0=1;
//	RTS=1;	// 1:送信しないでください 0:送信してください
	
	mstuart=0;			// モジュールスタンバイ解除
	u0mr=0b00000101;	// 8ビット ストップビット1 パリティ無し
	u0c0=0b00010000;	// LSBファースト プッシュプル出力 フィルタON カウントソース分周無し
	u0brg=119;			// 9600bps
	//u0brg=29;			// 38400bps
	//u0brg=19;			// 57600bps
	//u0brg=9;			// 115200bps
	u0rrm_u0c1=0;		// 連続受信モード禁止
	u0tie=0;			// 送信割り込み禁止
	u0rie=0;			// 受信割り込み禁止
	te_u0c1=1;			// 送信許可
	re_u0c1=1;			// 受信許可

	/*ilvl8=(ilvl8 & 0x0F) | (1 << 4);	// 受信割り込みレベル1
	u0rie=1;			// 受信割り込み許可
	
	readpos=0;
	writepos=0;
	
	asm("LDIPL #0");	// プロセッサ割り込み優先レベル0（この値よりも高いレベルの割り込みが受け付けられる）
	asm("FSET I");		// 割り込み許可
	RTS=0;				// 送信要求
	*/
}	

void putch(unsigned char a)
{
	while(ti_u0c1==0);	// バッファが空になるまで待つ
	
	u0tbh=0;
	u0tbl=a;
}	

void puts(unsigned char str[])
{
	unsigned int a=0;
	while(1)
	{
		if(str[a]==0) break;
		putch(str[a]);
		a++;
	}
}

void puthex(unsigned char a)
{
	putch(tochar((a&0xF0)>>4));
	putch(tochar(a&0x0F));
}

unsigned char tochar(unsigned char a)
{
	if(a<10) a=a+0x30;
	else a=a+0x41-10;
	return a;
}

void puthexshort(unsigned short a)
{
	puthex((a>>8)&0xFF);
	puthex((a&0xFF));
}

// なにか受信したら1を返す
unsigned char keyhit(void)
{
	return (unsigned char)ri_u0c1;
}

// 割り込みを使わない1バイトリード
unsigned char getch(void)
{
//	RTS=0;	// 送信要求
	while(ri_u0c1==0);	// 受信するまで待つ
//	RTS=1;	// 送信禁止
	return (unsigned char)(u0rb&0xFF);	// u0rbを読むとriは0になる
}

// 割り込みを使う1バイトリード 受信バッファ付き
/*unsigned char getch(void)
{
	unsigned char a;
	while(writepos==readpos); // 受信するまで待つ
	a=buff[readpos];
	readpos++;
	readpos&=(buffsize-1);
	// 受信バッファが空になったら送信要求
	if(writepos==readpos) RTS=0;	// 送信要求
	
	return a;
}*/


unsigned short getshort(void)
{
	unsigned short data;
	data =((unsigned short)getch()<<8);
	data+=((unsigned short)getch()   );
	return data;
}

void putshort(unsigned short data)
{
	putch((data>>8)&0xFF);
	putch((data   )&0xFF);
}

void putdecimal(unsigned short data)
{
	putch(tochar(data/10000));
	data=data%10000;
	putch(tochar(data/1000));
	data=data%1000;
	putch(tochar(data/100));
	data=data%100;
	putch(tochar(data/10));
	data=data%10;
	putch(tochar(data));
}

/*
#pragma INTERRUPT int_uart_receive	// int_uart_receive 関数は割り込みルーチンですよとコンパイラにお知らせ
void int_uart_receive(void)
{
	signed char a;
	// 割り込みフラグクリア
	while(u0rif==1) u0rif==0;

	buff[writepos]=(unsigned char)u0rb;
	writepos++;
	writepos&=(buffsize-1);
	
	// バッファに溜まってきたら送信禁止にする
	// RTSを1にしても、すぐには止まらないので閾値は余裕も持つ
	a=writepos-readpos;
	if(a>0)
	{
		if(a<4) RTS=1;	// 送信禁止
	}
	else
	{
		if(a>-4) RTS=1;	// 送信禁止
	}
}
*/