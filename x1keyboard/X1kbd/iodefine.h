/*
	PS/2 キーボードを SHARP X1 につなぐ
	I/O ピンの定義ほか

	2014年7月22日 作成
	
	佐藤恭一  http://kyoutan.jpn.org/

	無保証です。
	佐藤恭一が作成した部分は用途に制限を設けません。商用・非商用にかかわらず自由に使用して頂いて構いません。
	勝手に複製したり、改造したり、配布したり、売ったりしても良いということです。
	連絡不要です。
*/

#define PS2DATA		p3_3
#define X1KEYOUT	p3_7

#define	TRUE	1
#define	FALSE	0
#define	NULL	0

#define DI()	asm("FCLR I")	// 割り込み禁止
#define EI()	asm("FSET I")	// 割り込み許可
