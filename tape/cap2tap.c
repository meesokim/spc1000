#include <stdio.h>
#include <string.h>

//#define DEBUG
void main(int argc, char **argv)
{
    FILE *f;
    if (argc > 1)
        f = fopen(argv[1], "r");
    else
        f = stdin;
    int minus = 0;
    int count = 0;
    int zero = 0;
    int plus = 0;
    char a;
    char str[4];
    memset(str, 0, 4);
    str[3] = 0;
    int pos = 0;
    while(!feof(f))
    {
        a = getc(f);
        if (a != '0' || str[2] != '0')
        {
            str[0] = str[1];
            str[1] = str[2];
            str[2] = a;
            if (!strcmp(str, "-0+") || !strcmp(str, "--+") || !strcmp(str, "-++"))
            {
                count = 1;
                minus = 0;
            }
            else if (a == '-')
                minus++;
            if (count > 0 && minus > 3 && plus > 4)
            {
                count = 0;
            }
            // if (count)
#ifdef DEBUG                
                putchar(a);
#endif                
            //printf("%s%d+%d,0=%d\n", str, count, plus, zero);
            if (count)
            {
                if (a != '-')
                    plus++;
                if (a == '0')
                    zero++;
            }
            else 
            {
                if (zero < 6)
                {
                    if (plus > 16 && plus < 30)
                    {
#ifdef DEBUG   
                        printf(">1(%d,%d)\n", plus, pos++);
#else                        
                        printf("1");
#endif                    
                    }
                    else if (plus > 4 && plus < 17)
                    {
#ifdef DEBUG   
                        printf(">0(%d,%d)\n", plus, pos++);
#else                        
                        printf("0");
#endif                    
                    }
                    else if (plus > 0)
                    {
#ifdef DEBUG   
                        printf(">x(%d)\n", plus);
#else                        
                        printf("x");
#endif                    
                    }
                }
                plus = 0;
                zero = 0;
            }
        }
        else if (count)
        {
            plus++;
        }
    }
}