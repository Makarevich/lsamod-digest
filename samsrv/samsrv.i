
long WINAPI SamrCloseHandle( 
  SAMPR_HANDLE* SamHandle 
) SAM_POSTFIX

long WINAPI SamrOpenDomain( 
  SAMPR_HANDLE ServerHandle, 
  unsigned long DesiredAccess, 
  PRPC_SID DomainId, 
  SAMPR_HANDLE* DomainHandle 
) SAM_POSTFIX

long WINAPI SamIConnect( 
  PSAMPR_SERVER_NAME ServerName, 
  SAMPR_HANDLE* ServerHandle, 
  unsigned long ClientRevision,
  unsigned long DesiredAccess 
) SAM_POSTFIX

long WINAPI SamrEnumerateUsersInDomain( 
  SAMPR_HANDLE DomainHandle, 
  unsigned long* EnumerationContext, 
  unsigned long UserAccountControl, 
  PSAMPR_ENUMERATION_BUFFER* Buffer, 
  unsigned long PreferedMaximumLength, 
  unsigned long* CountReturned 
) SAM_POSTFIX


long WINAPI SamrQueryInformationDomain( 
  SAMPR_HANDLE DomainHandle, 
  DOMAIN_INFORMATION_CLASS DomainInformationClass, 
  PSAMPR_DOMAIN_INFO_BUFFER* Buffer 
) SAM_POSTFIX


NTSTATUS WINAPI SamIGetUserLogonInformation(
        SAMPR_HANDLE sam_domain,
        unsigned long flags,
        PUNICODE_STRING username,
        PSAMPR_USER_INFO_BUFFER *ppuser_info,
        PSidAndAttributesList psidattr,
        SAMPR_HANDLE *sam_user) SAM_POSTFIX

NTSTATUS WINAPI SamIGetUserLogonInformationEx(
        SAMPR_HANDLE sam_domain,
        unsigned long flags,
        PUNICODE_STRING username,
        unsigned long flags2,
        PSAMPR_USER_INFO_BUFFER *ppuser_info,
        PSidAndAttributesList psidattr,
        SAMPR_HANDLE *sam_user) SAM_POSTFIX

NTSTATUS WINAPI SamIRetrievePrimaryCredentials(
        SAMPR_HANDLE sam_user,
        PUNICODE_STRING cred,
        LPVOID *outbuf,
        long *outbufsz) SAM_POSTFIX

NTSTATUS WINAPI SamIFreeSidAndAttributesList(PSidAndAttributesList psidattr) SAM_POSTFIX

NTSTATUS WINAPI SamIFree_SAMPR_USER_INFO_BUFFER(PSAMPR_USER_INFO_BUFFER pbuf, USER_INFORMATION_CLASS bufcl) SAM_POSTFIX
NTSTATUS WINAPI SamIFree_SAMPR_DOMAIN_INFO_BUFFER(PSAMPR_DOMAIN_INFO_BUFFER pbuf, DOMAIN_INFORMATION_CLASS bufcl) SAM_POSTFIX
NTSTATUS WINAPI SamIFree_SAMPR_ENUMERATION_BUFFER(PSAMPR_ENUMERATION_BUFFER pbuf) SAM_POSTFIX

