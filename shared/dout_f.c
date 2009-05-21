#include <windows.h>
#include "dout.h"

#define LOG_FILE_DIR    "C:\\"

const char dout_file[];


static HANDLE   h_file = INVALID_HANDLE_VALUE;



static HANDLE open_log_file(){
    char    buf[64] = LOG_FILE_DIR;
    HANDLE  h;

    lstrcat(buf, dout_file);

    h = CreateFile(buf, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(h == INVALID_HANDLE_VALUE){
        return INVALID_HANDLE_VALUE;
    }

    if(SetFilePointer(h, 0, NULL, FILE_END) == INVALID_SET_FILE_POINTER){
        CloseHandle(h);
        return INVALID_HANDLE_VALUE;
    }

    return h;
}

void dout(const char* message){
    const char      *s;
    char            c, *d, *e;
    char            buf[256];
    DWORD           dw;

    if(h_file == INVALID_HANDLE_VALUE){
        h_file = open_log_file();
        if(h_file == INVALID_HANDLE_VALUE){
            return;
        }
    }

    s = message;
    d = buf;
    e = buf + (sizeof(buf) / sizeof(buf[0]));

    for(;;){
        if((e - d) <= 2){
            WriteFile(h_file, buf, (d - buf) * sizeof(buf[0]), &dw, NULL);
            d = buf;
        }

        c = *(s++);
        switch(c){
        case '\0':
            WriteFile(h_file, buf, (d - buf) * sizeof(buf[0]), &dw, NULL);
            goto double_break;
        case '\n':
        case '\r':
            *(d++) = '\r';
            *(d++) = '\n';
            break;
        default:
            *(d++) = c;
        }
    }

double_break:;
}

void dout_close(){
    CloseHandle(h_file);
    h_file = INVALID_HANDLE_VALUE;
}


