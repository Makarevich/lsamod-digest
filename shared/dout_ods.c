#include <windows.h>
#include "dout.h"

void dout(const char* message){
    OutputDebugString(message);
}

void dout_close(){
}

