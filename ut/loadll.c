
//
// loadll.c
//
//   This program simply loads a DLL into its address space.
//
//   You specify a library name in the command line.
//   It calls LoadLibrary against the 1st parameter.
//   When you press Ctrl+C, it calls FreeLibrary and exits.
//


#include <windows.h>
#include "../shared/shared.h"

void show_help(){
    dout("Usage: loadll.exe mudll.dll\n");
}

HMODULE open_dll(){
    char        *cmd;
    char        buf[1024];
    char        *p;
    HMODULE     h;

    //////////////////////////////

    cmd = GetCommandLine();

    while(*cmd == ' ') cmd++;

#define ZEROCHK         if(*cmd == 0){ show_help(); return NULL; }

    if(*cmd == '\"'){
        cmd++;
        while(*cmd != '\"'){
            ZEROCHK;
            cmd++;
        }
        cmd++;
    }else{
        while(*cmd != ' '){
            ZEROCHK;
            cmd++;
        }
    }

    while(*cmd == ' ') cmd++;

    ZEROCHK;
#undef ZEROCHK

    p = buf;
    if(*cmd == '\"'){
        cmd++;
        while(*cmd != '\"'){
            if(*cmd == 0) break;
            *(p++) = *(cmd++);
        }
    }else{
        while(*cmd != ' '){
            if(*cmd == 0) break;
            *(p++) = *(cmd++);
        }
    }
    *p = 0;

    //////////////////////////////

    dout(va("Opening \"%s\"...\n", buf));

    if((h = LoadLibrary(buf)) == NULL){
        dout(va("LoadLibrary failed: %u\n", GetLastError()));
        return NULL;
    }
    
    return h;
}

///////////////////////////////////////

static volatile int break_all = 0;

BOOL WINAPI handler(DWORD code){
    if(code == CTRL_C_EVENT){
        break_all = 1;
        return TRUE;
    }

    return FALSE;
}

int main(){
    HMODULE h;

    if(!SetConsoleCtrlHandler(handler, TRUE)){
        dout(va("WARNING: SetConsoleCtrlHandler failed: %u\n", GetLastError()));
    }

    if(!(h = open_dll())){
        return -1;
    }

    dout("The DLL has been loaded. Press Ctrl+C to exit.\n");

    while(!break_all) Sleep(100);

    dout("...");

    FreeLibrary(h);

    dout("...\n");

    return 0;
}

