
#ifndef __UTILS_H__
#define __UTILS_H__

char *va(const char* msg, ... );

int unicode2ansi(LPCWSTR src, int sc, LPSTR dst, int ds);



#define HASHLEN 16
typedef unsigned char HASH[HASHLEN];
#define HASHHEXLEN 32
typedef char HASHHEX[HASHHEXLEN + 1];

void CvtHex(const HASH Bin, HASHHEX Hex);
void CvtBin(const HASHHEX Hex, HASH Bin);

#endif // __UTILS_H__

