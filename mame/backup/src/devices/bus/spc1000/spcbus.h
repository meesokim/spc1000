
#define SPCREAD "spcread"
#define SPCWRITE "spcwrite"
#define SPCRESET "reset"
#define SPCINIT "init"

typedef unsigned char (*ReadfnPtr)(int, unsigned short);
typedef void (*WritefnPtr)(int, unsigned short, unsigned char);
typedef void (*InitfnPtr)(char *);
typedef void (*ResetfnPtr)(void);
