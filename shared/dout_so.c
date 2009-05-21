#include <windows.h>
#include "dout.h"

void dout(const char* message){
    DWORD dw;
    WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), message, lstrlen(message), &dw, 0);
}

void dout_close(){
}

