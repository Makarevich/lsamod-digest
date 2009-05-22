

#include "lm_sam.h"
#include "../samsrv/samsrv.h"

#define DOUTST2(api, status)        DOUTST("lm_sam", api, status)

#define SAM_CONNECTION_TIMEOUT      5000        // the timeout, after which the sam connection is closed and the object is transferred
                                                // from state 3 to state 2

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS    0
#endif 

int onthink_dummy(struct lm_sam_s *This);
int ondata_no_sid(struct lm_sam_s *This, PUNICODE_STRING uname, HASH hash, NTSTATUS *result);
int onterminate_no_sid(struct lm_sam_s *This);
int ondata_no_sam(struct lm_sam_s *This, PUNICODE_STRING uname, HASH hash, NTSTATUS *result);
int onterminate_no_sam(struct lm_sam_s *This);
int onthink_sam_opened(struct lm_sam_s *This);
int ondata_sam_opened(struct lm_sam_s *This, PUNICODE_STRING uname, HASH hash, NTSTATUS *result);
int onterminate_sam_opened(struct lm_sam_s *This);

lm_sam_state_t          state_no_sid = {onthink_dummy, ondata_no_sid, onterminate_no_sid};
lm_sam_state_t          state_no_sam = {onthink_dummy, ondata_no_sam, onterminate_no_sam};
lm_sam_state_t          state_sam_opened = {onthink_sam_opened, ondata_sam_opened, onterminate_sam_opened};

void init_lm_sam(lm_sam_t* This){
    This->lsa_policy_info_buffer = NULL;
    This->domain_sid = NULL;

    This->sam_server = NULL;
    This->sam_domain = NULL;

    This->last_sam_activity = 0;

    This->state = &state_no_sid;
}

//
// state 1: no_sam (no domain SID)
//   onthink:       do nothing
//   ondata:        obtain account domain SID, change state to no_sam, and retry
//   onterminate:   do nothing
//

static int onthink_dummy(struct lm_sam_s *This){
    return 1;
}

static int ondata_no_sid(struct lm_sam_s *This, PUNICODE_STRING uname, HASH hash, NTSTATUS *result){
    LSA_HANDLE                      h_policy;
    LSA_OBJECT_ATTRIBUTES           objattr;
    POLICY_ACCOUNT_DOMAIN_INFO      *pdomain_info;
    NTSTATUS                        status;
    char                            dname[64];

    memset(&objattr, 0, sizeof(objattr));
    objattr.Length = sizeof(objattr);

    if((status = LsaOpenPolicy(NULL, &objattr, POLICY_VIEW_LOCAL_INFORMATION, &h_policy)) != STATUS_SUCCESS){
        DOUTST2("LsaOpenPolicy", status);
        *result = status;
        return 0;
    }

    if((status = LsaQueryInformationPolicy(h_policy, PolicyAccountDomainInformation, &pdomain_info)) != STATUS_SUCCESS){
        DOUTST2("LsaQueryInformationPolicy", status);
        LsaClose(h_policy);
        *result = status;
        return 0;
    }

    if(unicode2ansi(pdomain_info->DomainName.Buffer, pdomain_info->DomainName.Length, dname, sizeof(dname)) == 0){
        strcpy(dname, "<unknown>");
    }

    dout(va("Current domain is %s.\n", dname));

    This->lsa_policy_info_buffer = pdomain_info;
    This->domain_sid = pdomain_info->DomainSid;

    // delegate processing to no_sam state
    This->state = &state_no_sam;
    return This->state->data(This, uname, hash, result);
}

static int onterminate_no_sid(struct lm_sam_s *This){
    return 1;
}

//
// state 2: no_sam (SID is resolved, but the SAM server is not opened)
//   onthink:       do nothing
//   ondata:        open the SAM server, delegate processing to sam_opened state
//   onterminate:   free the account domain SID, delegate to no_sid state
//

static int ondata_no_sam(struct lm_sam_s *This, PUNICODE_STRING uname, HASH hash, NTSTATUS *result){
    SAMPR_HANDLE                    ssrv = NULL;
    SAMPR_HANDLE                    sdomain = NULL;
    NTSTATUS                        status;

    dout("Opening sam...\n");

    if((status = SamIConnect(NULL, &ssrv, 0, SAM_SERVER_CONNECT)) != STATUS_SUCCESS){
        DOUTST2("SamIConnect", status);
        *result = status;
        return 0;
    }

    // FIXME: specify proper DesiredAccess in a call to SamrOpenDomain
    if((status = SamrOpenDomain(ssrv, DOMAIN_LOOKUP | DOMAIN_LIST_ACCOUNTS | DOMAIN_READ_OTHER_PARAMETERS, This->domain_sid, &sdomain)) != STATUS_SUCCESS){
        DOUTST2("SamrOpenDomain", status);
        *result = status;
        SamrCloseHandle(ssrv);
        return 0;
    }

    // dout("Sam has been opened\n");

    This->sam_server = ssrv;
    This->sam_domain = sdomain;

    // delegate processing to sam_opened state
    This->state = &state_sam_opened;
    return This->state->data(This, uname, hash, result);
}

static int onterminate_no_sam(struct lm_sam_s *This){
    LsaFreeMemory(This->lsa_policy_info_buffer);
    This->lsa_policy_info_buffer = NULL;
    This->domain_sid = NULL;

    This->state = &state_no_sid;
    return This->state->terminate(This);
}

//
// state 3: sam_opened (SID is resolved, the SAM server is opened)
//   onthink:       on timeout close the SAM server and transfer to no_sam state
//   ondata:        call SamIGetUserLogonInformationEx on the specified username
//   onterminate:   close the SAM server and delegate to no_sam state
//

static void close_sam(struct lm_sam_s *This){
    SamrCloseHandle(This->sam_domain);
    SamrCloseHandle(This->sam_server);
    This->sam_domain = 0;
    This->sam_server = 0;
}

static int onthink_sam_opened(struct lm_sam_s *This){
    DWORD       now = GetTickCount();
    DWORD       then = This->last_sam_activity;

    if((now - then) > SAM_CONNECTION_TIMEOUT){
        dout("Closing sam due to timeout\n");

        close_sam(This);
        This->state = &state_no_sam;
    }

    return 1;
}

static int ondata_sam_opened(struct lm_sam_s *This, PUNICODE_STRING uname, HASH hash, NTSTATUS *result){
    WCHAR                       w_password[] = L"WDigest";
    UNICODE_STRING              u_password = {sizeof(w_password) - sizeof(WCHAR), sizeof(w_password) - sizeof(WCHAR), w_password};
    char                        astr[64];
    PSAMPR_USER_INFO_BUFFER     puser_info_buffer;
    SidAndAttributesList        sidattr;
    SAMPR_HANDLE                sam_user;
    PWDIGEST_CREDENTIALS        pwdigest_creds;
    long                        cred_sz;
    NTSTATUS                    status;

    if(unicode2ansi(uname->Buffer, uname->Length, astr, sizeof(astr)) == 0){
        dout("ERROR: unicode2ansi failed in ondata_sam_opened\n");
        strcpy(astr, "<unknown>");
    }

    dout(va("Resolving user \"%s\"\n", astr));

    This->last_sam_activity = GetTickCount();

    // open sam account here
    status = SamIGetUserLogonInformationEx(This->sam_domain, 0, uname, /*0x1BE40D*/0, &puser_info_buffer, &sidattr, &sam_user);
    if(status != STATUS_SUCCESS){
        DOUTST2("SamIGetUserLogonInformationEx", status);
        *result = status;
        return 0;
    }

    SamIFree_SAMPR_USER_INFO_BUFFER(puser_info_buffer, UserAllInformation);
    SamIFreeSidAndAttributesList(&sidattr);

#if 0
    *result = STATUS_SUCCESS;
    memcpy(hash, "\x12\x34\x56\x78" "\x11\x11\x11\x11" "\xaa\xCC\xaa\xaa" "\xbb\xbb\xbb\xbb", HASHLEN);

#else
    status = SamIRetrievePrimaryCredentials(sam_user, &u_password, &pwdigest_creds, &cred_sz);
    if(status != STATUS_SUCCESS){
        DOUTST2("SamIRetrievePrimaryCredentials", status);
        *result = status;
    }else{
        if(cred_sz != sizeof(WDIGEST_CREDENTIALS)){
            dout(va("WDigest credentials size mismatrch: %i (%08X) != %i\n", cred_sz, cred_sz, sizeof(WDIGEST_CREDENTIALS)));
            *result = STATUS_INTERNAL_ERROR;
        }else{
            memcpy(hash, pwdigest_creds->Hash1, HASHLEN);

            if(1){
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

                dout(
                    "WDIGEST_CREDENTIALS\n"
                );
            }

            *result = STATUS_SUCCESS;
        }

        LocalFree(pwdigest_creds);
    }
#endif
    
    SamrCloseHandle(sam_user);

    return (*result == STATUS_SUCCESS)?1:0;
}

static int onterminate_sam_opened(struct lm_sam_s *This){
    close_sam(This);
    This->state = &state_no_sam;
    return This->state->terminate(This);
}



