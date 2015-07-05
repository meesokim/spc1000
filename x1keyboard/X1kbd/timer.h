/*
	PS/2 キーボードを SHARP X1 につなぐ
	100[ms] のインターバルタイマー

	2014年7月22日 作成
	
	佐藤恭一  http://kyoutan.jpn.org/

	無保証です。
	佐藤恭一が作成した部分は用途に制限を設けません。商用・非商用にかかわらず自由に使用して頂いて構いません。
	勝手に複製したり、改造したり、配布したり、売ったりしても良いということです。
	連絡不要です。
*/

extern volatile unsigned short TIMER;		// 100[ms]タイマー

void timer_init(void);
//void timer_start(void);
#define timer_start()	tstart_trbcr=1	// TIMER RB2 カウント開始 
