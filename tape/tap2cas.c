#include <stdio.h>
#include <string.h>
#ifdef _WIN32
  #include <io.h>
  #include <fcntl.h>
#endif
//#define DEBUG
int main(int argc, char **argv)
{
    FILE *f;
    if (argc > 1)
        f = fopen(argv[1], "r");
    else
        f = stdin;
#ifdef _WIN32
  setmode(fileno(stdout),O_BINARY);
#else
    freopen(NULL, "wb", stdout);
#endif    
    printf("SPC-1000.CASfmt ");
    char a;
    unsigned char b = 0;
    int pos = 8;
    while(!feof(f))
    {
        a = getc(f);
//        printf("%c", a);
        if (a == '1' || a == '0')
        {            
            pos--;
            b += (a == '1' ? 1 << pos : 0);
            if (pos == 0)
            {
    //            printf(":%02x\n", b);
                putc(b, stdout);
                b = 0;
                pos = 8;
            }
        }
    }
    return 0;
}