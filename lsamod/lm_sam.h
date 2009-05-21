

#ifndef __LM_SAM_H__
#define __LM_SAM_H__

#include <windows.h>
#include <ntsecapi.h>

#include "../shared/shared.h"
#include "../samsrv/samsrv.h"

typedef int (lm_sam_onthink_t*)(struct lm_sam_s *This);
typedef int (lm_sam_ondata_t*)(struct lm_sam_s *This, PUNICODE_STRING uname, HASH hash, NTSTATUS *result);
typedef int (lm_sam_onterminate_t*)(struct lm_sam_s *This);

typedef struct {
    lm_sam_onthink_t        think;
    lm_sam_ondata_t         data;
    lm_sam_onterminate_t    terminate;
} lm_sam_state_t;

typedef struct lm_sam_s {
    lm_sam_state_t      *state;

    PVOID               lsa_policy_info_buffer;
    PSID                domain_sid;

    SAMPR_HANDLE        sam_server;
    SAMPR_HANDLE        sam_domain;

    DWORD               last_sam_activity;
} lm_sam_t;

void init_lm_sam(lm_sam_t* This);

#endif // __LM_SAM_H__

