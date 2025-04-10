#ifndef spc_call_h
#define spc_call_h

#if defined(_MSC_VER) || defined(_MINGW32) || defined(__MINGW32__) || defined(WIN32) || defined(WIN64)
#define RTLD_GLOBAL 0x100 /* do not hide entries in this module */
#define RTLD_LOCAL  0x000 /* hide entries in this module */
#define RTLD_LAZY   0x000 /* accept unresolved externs */
#define RTLD_NOW    0x001 /* abort if module has unresolved externs */
    #include <windows.h>
    #define SPC_BUS "spcbus.zxw"
    #define SPC_DRIVE "spcdrive.zxw"        
#elif defined(__GNUC__)
#if !defined(RASPPI)
    #include <dlfcn.h>
#endif
    #define SPC_BUS "./spcbus.zxl"    
    #define SPC_DRIVE "./spcdrive.zxl"
#else
    #error define your compiler
#endif

extern "C" {
void *OpenSPC(char *pcDllname, int iMode);
void *GetSPCFunc(void *Lib, char *Fnname);
int CloseSPC(void *hDLL);
char *GetSPCError();
}
#endif
