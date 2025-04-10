#include "spc.h"
#include "spcbus.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define LINES "-----------------------------------------------------\n"
using namespace std;

ReadfnPtr spcread = NULL;
WritefnPtr spcwrite = NULL;
InitfnPtr spcinit = NULL;
ResetfnPtr spcreset = NULL;

int main(int argc, char **argv)
{
    char *error;
    printf("%s\n", SPC_DRIVE);
    void *hDLL = OpenSPC((char*)SPC_DRIVE, RTLD_LAZY);
    if (!hDLL)
    {
        printf("DLL open error!!\n");
        exit(1);
    }
    spcreset = (ResetfnPtr)GetSPCFunc(hDLL, (char*)SPCRESET);
    spcread = (ReadfnPtr)GetSPCFunc(hDLL, (char*)SPCREAD);
    spcwrite = (WritefnPtr)GetSPCFunc(hDLL, (char*)SPCWRITE);
    spcinit = (InitfnPtr)GetSPCFunc(hDLL, (char*)SPCINIT);
    char *path = getenv("SPC_PATH");
    if (spcinit)
    {
        if (!path)
        {
            path = ".";
        }
        spcinit(path);

    }
    spcreset();
    if (argc > 1)
    {
        spcread(0x1);
    } 
    else// if (read)
    {
    }
    CloseSPC(hDLL);
    return 0;
}
