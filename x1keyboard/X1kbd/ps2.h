/*
	PS/2 キーボードを SHARP X1 につなぐ
	PS/2 キーボードの受信処理

	2014年7月22日 作成
	
	佐藤恭一  http://kyoutan.jpn.org/

	無保証です。
	佐藤恭一が作成した部分は用途に制限を設けません。商用・非商用にかかわらず自由に使用して頂いて構いません。
	勝手に複製したり、改造したり、配布したり、売ったりしても良いということです。
	連絡不要です。
*/

#define PS2TIMEOUT	30		// PS2 タイムアウト 30*100[ms] = 3[s]
#define PS2BUFFSIZE 0x10

extern volatile unsigned short PS2TIMER;		// PS2受信タイムアウトタイマー
//extern volatile unsigned char PS2BUFF[PS2BUFFSIZE];	// PS2受信バッファ
//extern volatile unsigned char PS2RPOS;		// PS2読み出し位置
//extern volatile unsigned char PS2WPOS;		// PS2書き込み位置

void ps2key_init(void);
unsigned char ps2size(void);
void ps2clear(void);
unsigned char ps2read(void);
unsigned char ps2get(void);
