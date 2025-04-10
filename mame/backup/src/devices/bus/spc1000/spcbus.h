
#define SPCREAD "spcread"
#define SPCWRITE "spcwrite"
#define SPCRESET "spcreset"
#define SPCINIT "spcinit"

typedef unsigned char (*ReadfnPtr)(unsigned short);
typedef void (*WritefnPtr)(unsigned short, unsigned char);
typedef void (*InitfnPtr)(char *);
typedef void (*ResetfnPtr)(void);
