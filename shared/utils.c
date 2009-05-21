#include <windows.h>
#include "utils.h"

static char * va_std_buffer = 0;

char *va(const char* msg, ... ){
    va_list list;

    if(va_std_buffer == 0){
        va_std_buffer = VirtualAlloc(NULL, 4000, MEM_COMMIT, PAGE_READWRITE);
    }

    va_start(list, msg);
    wvsprintf(va_std_buffer, msg, list);
    va_end(list);

    return va_std_buffer;
}


int ansi2unicode(LPCSTR src, int sc, LPWSTR dst, int ds){
    // return MultiByteToWideChar(CP_ACP, 0, src, sc, dst, ds);
    return 0;
}

int unicode2ansi(LPCWSTR src, int sc, LPSTR dst, int ds){
    int len = WideCharToMultiByte(CP_ACP, 0, src, sc / sizeof(WCHAR), dst, ds, 0, NULL);
    if(len > 0){
        if(len < ds){
            dst[len] = 0;
        }else{
            dst[ds - 1] = 0;
        }
    }

    return len;
}



void CvtHex(const HASH Bin, HASHHEX Hex) {
    unsigned short i;
    unsigned char j;

    for (i = 0; i < HASHLEN; i++) {
	j = (Bin[i] >> 4) & 0xf;
	if (j <= 9)
	    Hex[i * 2] = (j + '0');
	else
	    Hex[i * 2] = (j + 'a' - 10);
	j = Bin[i] & 0xf;
	if (j <= 9)
	    Hex[i * 2 + 1] = (j + '0');
	else
	    Hex[i * 2 + 1] = (j + 'a' - 10);
    }
    Hex[HASHHEXLEN] = '\0';
}

void CvtBin(const HASHHEX Hex, HASH Bin) {
    unsigned short i;
    unsigned char j;

    for (i = 0; i < HASHHEXLEN; i++) {
	unsigned char n;
	j = Hex[i];
	if (('0' <= j) && (j <= '9'))
	    n = j - '0';
	else if (('a' <= j) && (j <= 'f'))
	    n = j - 'a' + 10;
	else if (('A' <= j) && (j <= 'F'))
	    n = j - 'A' + 10;
	else
	    continue;
	if (i % 2 == 0)
	    Bin[i / 2] = n << 4;
	else
	    Bin[i / 2] |= n;
    }
    for (; i <= HASHHEXLEN; i++) {
        Bin[i] = '\0';
    }
}



