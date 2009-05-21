#include <windows.h>
#include "../shared/shared.h"
#include "../shared/md5.h"



const char dout_file[] = "ut1.log";


char* HA1(const char* user, const char* realm, const char* pwd){
    static HASHHEX      hex;
    HASH                h;
    SquidMD5_CTX        Md5Ctx;

    // dout(va("Hashing \"%s:%s:%s\"\n", user, realm, pwd));

    SquidMD5Init(&Md5Ctx);
	SquidMD5Update(&Md5Ctx, user, strlen(user));
	SquidMD5Update(&Md5Ctx, ":", 1);
	SquidMD5Update(&Md5Ctx, realm, strlen(realm));
	SquidMD5Update(&Md5Ctx, ":", 1);
	SquidMD5Update(&Md5Ctx, pwd, strlen(pwd));
	SquidMD5Final(h, &Md5Ctx);

    CvtHex(h, hex);
    return (char*)hex;
}

int main(){
    const char      user[] = "user2";
    const char      realm[] = "HOME";
    const char      pwd[] = "1234qwer@";

    char            user_u[32];
    char            user_l[32];
    char            realm_u[32];
    char            realm_l[32];

    strcpy(user_u, user);
    strcpy(user_l, user);
    _strupr(user_u);
    _strlwr(user_l);
    strcpy(realm_u, realm);
    strcpy(realm_l, realm);
    _strupr(realm_u);
    _strlwr(realm_l);

    // Hash1: MD5(sAMAccountName, NETBIOSDomainName, password) 
    // Hash2: MD5(UPPER(sAMAccountName), UPPER(NETBIOSDomainName), password) 
    // Hash3: MD5(LOWER(sAMAccountName), LOWER(NETBIOSDomainName), password) 
    // Hash4: MD5(sAMAccountName, UPPER(NETBIOSDomainName), password) 
    // Hash5: MD5(sAMAccountName, LOWER(NETBIOSDomainName), password) 
    // Hash6: MD5(UPPER(sAMAccountName), LOWER(NETBIOSDomainName), password) 
    // Hash7: MD5(LOWER(sAMAccountName), UPPER(NETBIOSDomainName), password) 

#define DO(n, u, r, p)      dout(va(#n ": HA1(%s,%s,%s) = %s\n", u, r, p, HA1(u, r, p)))

    DO(1, user, realm, pwd);
    DO(2, user_u, realm_u, pwd);
    DO(3, user_l, realm_l, pwd);
    DO(4, user, realm_u, pwd);
    DO(5, user, realm_l, pwd);
    DO(6, user_u, realm_l, pwd);
    DO(7, user_l, realm_u, pwd);

#undef DO



    return 0;
}

