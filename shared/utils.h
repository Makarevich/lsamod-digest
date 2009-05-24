
#ifndef __UTILS_H__
#define __UTILS_H__

char *va(const char* msg, ... );

int unicode2ansi(LPCWSTR src, int sc, LPSTR dst, int ds);
int ansi2unicode(LPCSTR src, int sc, LPWSTR dst, int ds);



#define HASHLEN 16
#define HASHHEXLEN 32

typedef unsigned char HASH[HASHLEN];
typedef char HASHHEX[HASHHEXLEN + 1];

void CvtHex(const HASH Bin, HASHHEX Hex);
void CvtBin(const HASHHEX Hex, HASH Bin);

#endif // __UTILS_H__

