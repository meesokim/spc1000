
#define SPCREAD "spcread"
#define SPCWRITE "spcwrite"
#define SPCRESET "spcreset"
#define SPCINIT "spcinit"

typedef unsigned char (*ReadfnPtr)(int, unsigned short);
typedef void (*WritefnPtr)(int, unsigned short, unsigned char);
typedef void (*InitfnPtr)(char *);
typedef void (*ResetfnPtr)(void);
