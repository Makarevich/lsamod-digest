
#ifndef __DOUT_H__
#define __DOUT_H__

#define DOUTST(mod,m,status)    dout(va(mod "(%u) " m " failed: %08X (%u)\n", __LINE__, status, status))
#define DOUTGLE(mod,m)          DOUTST(mod, m, GetLastError())


void dout(const char* message);
void dout_close();

#endif // __DOUT_H__

