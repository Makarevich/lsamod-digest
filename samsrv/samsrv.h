

#ifndef __SAMSRV_H__
#define __SAMSRV_H__

#define WIN32_NO_STATUS 
#include <windows.h>
#undef WIN32_NO_STATUS

#include <ntsecapi.h>
#include <ntstatus.h>

/*
#define STATUS_ACCESS_DENIED                0xC0000022
#define STATUS_MORE_ENTRIES                 0x00000105
#define STATUS_SOME_NOT_MAPPED              0x00000107
#define STATUS_NONE_MAPPED                  0xC0000073
#define STATUS_WRONG_PASSWORD               0xC000006A
#define STATUS_ACCOUNT_LOCKED_OUT           0xC0000234
#define STATUS_GROUP_EXISTS                 0xC0000065
#define STATUS_USER_EXISTS                  0xC0000063
#define STATUS_LM_CROSS_ENCRYPTION_REQUIRED 0xC000017F
#define STATUS_NT_CROSS_ENCRYPTION_REQUIRED 0xC000015D
*/

#define SAM_SERVER_CONNECT              0x00000001
#define SAM_SERVER_SHUTDOWN             0x00000002
#define SAM_SERVER_INITIALIZE           0x00000004
#define SAM_SERVER_CREATE_DOMAIN        0x00000008
#define SAM_SERVER_ENUMERATE_DOMAINS    0x00000010
#define SAM_SERVER_LOOKUP_DOMAIN        0x00000020
#define SAM_SERVER_ALL_ACCESS           0x000F003F
#define SAM_SERVER_READ                 0x00020010
#define SAM_SERVER_WRITE                0x0002000E
#define SAM_SERVER_EXECUTE              0x00020021

#define DOMAIN_READ_PASSWORD_PARAMETERS 0x00000001
#define DOMAIN_WRITE_PASSWORD_PARAMS    0x00000002
#define DOMAIN_READ_OTHER_PARAMETERS    0x00000004
#define DOMAIN_WRITE_OTHER_PARAMETERS   0x00000008
#define DOMAIN_CREATE_USER              0x00000010
#define DOMAIN_CREATE_GROUP             0x00000020
#define DOMAIN_CREATE_ALIAS             0x00000040
#define DOMAIN_GET_ALIAS_MEMBERSHIP     0x00000080
#define DOMAIN_LIST_ACCOUNTS            0x00000100
#define DOMAIN_LOOKUP                   0x00000200
#define DOMAIN_ADMINISTER_SERVER        0x00000400
#define DOMAIN_ALL_ACCESS               0x000F07FF
#define DOMAIN_READ                     0x00020084
#define DOMAIN_ALL_WRITE                0x0002047A
#define DOMAIN_ALL_EXECUTE              0x00020301

#define UF_SCRIPT                                   0x00000001
#define UF_ACCOUNTDISABLE                           0x00000002
#define UF_HOMEDIR_REQUIRED                         0x00000008
#define UF_LOCKOUT                                  0x00000010
#define UF_PASSWD_NOTREQD                           0x00000020
#define UF_PASSWD_CANT_CHANGE                       0x00000040
#define UF_ENCRYPTED_TEXT_PASSWORD_ALLOWED          0x00000080
#define UF_TEMP_DUPLICATE_ACCOUNT                   0x00000100
#define UF_NORMAL_ACCOUNT                           0x00000200
#define UF_INTERDOMAIN_TRUST_ACCOUNT                0x00000800
#define UF_WORKSTATION_TRUST_ACCOUNT                0x00001000
#define UF_SERVER_TRUST_ACCOUNT                     0x00002000
#define UF_DONT_EXPIRE_PASSWD                       0x00010000
#define UF_MNS_LOGON_ACCOUNT                        0x00020000
#define UF_SMARTCARD_REQUIRED                       0x00040000
#define UF_TRUSTED_FOR_DELEGATION                   0x00080000
#define UF_NOT_DELEGATED                            0x00100000
#define UF_USE_DES_KEY_ONLY                         0x00200000
#define UF_DONT_REQUIRE_PREAUTH                     0x00400000
#define UF_PASSWORD_EXPIRED                         0x00800000
#define UF_TRUSTED_TO_AUTHENTICATE_FOR_DELEGATION   0x01000000
#define UF_NO_AUTH_DATA_REQUIRED                    0x02000000

#define USER_READ_GENERAL                           0x00000001
#define USER_READ_PREFERENCES                       0x00000002
#define USER_WRITE_PREFERENCES                      0x00000004
#define USER_READ_LOGON                             0x00000008
#define USER_READ_ACCOUNT                           0x00000010
#define USER_WRITE_ACCOUNT                          0x00000020
#define USER_CHANGE_PASSWORD                        0x00000040
#define USER_FORCE_PASSWORD_CHANGE                  0x00000080
#define USER_LIST_GROUPS                            0x00000100
#define USER_READ_GROUP_INFORMATION                 0x00000200
#define USER_WRITE_GROUP_INFORMATION                0x00000400
#define USER_ALL_ACCESS                             0x000F07FF
#define USER_ALL_READ                               0x0002031A
#define USER_ALL_WRITE                              0x00020044
#define USER_ALL_EXECUTE                            0x00020041

#ifndef SID_IDENTIFIER_AUTHORITY_DEFINED
#define SID_IDENTIFIER_AUTHORITY_DEFINED
typedef struct _SID_IDENTIFIER_AUTHORITY { 
  unsigned char Value[6]; 
} SID_IDENTIFIER_AUTHORITY,  
 *PSID_IDENTIFIER_AUTHORITY; 
#endif
 
typedef struct _RPC_SID { 
  unsigned char Revision; 
  unsigned char SubAuthorityCount; 
  SID_IDENTIFIER_AUTHORITY IdentifierAuthority; 
  unsigned long SubAuthority[]; 
} RPC_SID,  
 *PRPC_SID,  
 **PPRPC_SID; 



typedef void*               SAMPR_HANDLE, *PSAMPR_HANDLE; 
typedef UNICODE_STRING      RPC_UNICODE_STRING;
typedef RPC_UNICODE_STRING  SAMPR_SERVER_NAME, *PSAMPR_SERVER_NAME;

typedef struct _SAMPR_RID_ENUMERATION {
  unsigned long RelativeId;
  RPC_UNICODE_STRING Name;
} SAMPR_RID_ENUMERATION, *PSAMPR_RID_ENUMERATION;

typedef struct _SAMPR_ULONG_ARRAY {
  unsigned long Count;
  unsigned long* Element;
} SAMPR_ULONG_ARRAY, *PSAMPR_ULONG_ARRAY;

typedef struct _SAMPR_ENUMERATION_BUFFER {
  unsigned long EntriesRead;
  PSAMPR_RID_ENUMERATION Buffer;
} SAMPR_ENUMERATION_BUFFER,  *PSAMPR_ENUMERATION_BUFFER;


typedef  enum _USER_INFORMATION_CLASS { 
  UserGeneralInformation = 1, 
  UserPreferencesInformation = 2, 
  UserLogonInformation = 3, 
  UserLogonHoursInformation = 4, 
  UserAccountInformation = 5, 
  UserNameInformation = 6, 
  UserAccountNameInformation = 7, 
  UserFullNameInformation = 8, 
  UserPrimaryGroupInformation = 9, 
  UserHomeInformation = 10, 
  UserScriptInformation = 11, 
  UserProfileInformation = 12, 
  UserAdminCommentInformation = 13, 
  UserWorkStationsInformation = 14, 
  UserControlInformation = 16, 
  UserExpiresInformation = 17, 
  UserInternal1Information = 18, 
  UserParametersInformation = 20, 
  UserAllInformation = 21, 
  UserInternal4Information = 23, 
  UserInternal5Information = 24, 
  UserInternal4InformationNew = 25, 
  UserInternal5InformationNew = 26 
} USER_INFORMATION_CLASS,  
 *PUSER_INFORMATION_CLASS; 

typedef struct _OLD_LARGE_INTEGER { 
  unsigned long LowPart; 
  long HighPart; 
} OLD_LARGE_INTEGER,  
 *POLD_LARGE_INTEGER; 

typedef struct _SAMPR_SR_SECURITY_DESCRIPTOR { 
  unsigned long Length; 
  unsigned char* SecurityDescriptor; 
} SAMPR_SR_SECURITY_DESCRIPTOR,  
 *PSAMPR_SR_SECURITY_DESCRIPTOR; 

typedef struct _SAMPR_LOGON_HOURS { 
  unsigned short UnitsPerWeek; 
  unsigned char* LogonHours; 
} SAMPR_LOGON_HOURS,  
 *PSAMPR_LOGON_HOURS; 
 
 
typedef struct _SAMPR_USER_ALL_INFORMATION { 
  OLD_LARGE_INTEGER LastLogon; 
  OLD_LARGE_INTEGER LastLogoff; 
  OLD_LARGE_INTEGER PasswordLastSet; 
  OLD_LARGE_INTEGER AccountExpires; 
  OLD_LARGE_INTEGER PasswordCanChange; 
  OLD_LARGE_INTEGER PasswordMustChange; 
  RPC_UNICODE_STRING UserName; 
  RPC_UNICODE_STRING FullName; 
  RPC_UNICODE_STRING HomeDirectory; 
  RPC_UNICODE_STRING HomeDirectoryDrive; 
  RPC_UNICODE_STRING ScriptPath; 
  RPC_UNICODE_STRING ProfilePath; 
  RPC_UNICODE_STRING AdminComment; 
  RPC_UNICODE_STRING WorkStations; 
  RPC_UNICODE_STRING UserComment; 
  RPC_UNICODE_STRING Parameters; 
  RPC_UNICODE_STRING LmOwfPassword; 
  RPC_UNICODE_STRING NtOwfPassword; 
  RPC_UNICODE_STRING PrivateData; 
  SAMPR_SR_SECURITY_DESCRIPTOR SecurityDescriptor; 
  unsigned long UserId; 
  unsigned long PrimaryGroupId; 
  unsigned long UserAccountControl; 
  unsigned long WhichFields; 
  SAMPR_LOGON_HOURS LogonHours; 
  unsigned short BadPasswordCount; 
  unsigned short LogonCount; 
  unsigned short CountryCode; 
  unsigned short CodePage; 
  unsigned char LmPasswordPresent; 
  unsigned char NtPasswordPresent; 
  unsigned char PasswordExpired; 
  unsigned char PrivateDataSensitive; 
} SAMPR_USER_ALL_INFORMATION,  
 *PSAMPR_USER_ALL_INFORMATION; 
 

typedef  
union _SAMPR_USER_INFO_BUFFER { 
    /*
  SAMPR_USER_GENERAL_INFORMATION General; 
  SAMPR_USER_PREFERENCES_INFORMATION Preferences; 
  SAMPR_USER_LOGON_INFORMATION Logon; 
  SAMPR_USER_LOGON_HOURS_INFORMATION LogonHours; 
  SAMPR_USER_ACCOUNT_INFORMATION Account; 
  SAMPR_USER_NAME_INFORMATION Name; 
  SAMPR_USER_A_NAME_INFORMATION AccountName; 
  SAMPR_USER_F_NAME_INFORMATION FullName; 
  USER_PRIMARY_GROUP_INFORMATION PrimaryGroup; 
  SAMPR_USER_HOME_INFORMATION Home; 
  SAMPR_USER_SCRIPT_INFORMATION Script; 
  SAMPR_USER_PROFILE_INFORMATION Profile; 
  SAMPR_USER_ADMIN_COMMENT_INFORMATION AdminComment; 
  SAMPR_USER_WORKSTATIONS_INFORMATION WorkStations; 
  USER_CONTROL_INFORMATION Control; 
  USER_EXPIRES_INFORMATION Expires; 
  SAMPR_USER_INTERNAL1_INFORMATION Internal1; 
  SAMPR_USER_PARAMETERS_INFORMATION Parameters; 
    */
  SAMPR_USER_ALL_INFORMATION All; 
    /*
  SAMPR_USER_INTERNAL4_INFORMATION Internal4; 
  SAMPR_USER_INTERNAL5_INFORMATION Internal5; 
  SAMPR_USER_INTERNAL4_INFORMATION_NEW Internal4New; 
  SAMPR_USER_INTERNAL5_INFORMATION_NEW Internal5New; 
    */
} SAMPR_USER_INFO_BUFFER,  
 *PSAMPR_USER_INFO_BUFFER; 

/////////////////////////////

typedef  enum _DOMAIN_INFORMATION_CLASS 
{ 
  DomainPasswordInformation = 1, 
  DomainGeneralInformation = 2, 
  DomainLogoffInformation = 3, 
  DomainOemInformation = 4, 
  DomainNameInformation = 5, 
  DomainReplicationInformation = 6, 
  DomainServerRoleInformation = 7, 
  DomainModifiedInformation = 8, 
  DomainStateInformation = 9, 
  DomainUasInformation = 10, 
  DomainGeneralInformation2 = 11, 
  DomainLockoutInformation = 12, 
  DomainModifiedInformation2 = 13 
} DOMAIN_INFORMATION_CLASS; 

typedef  enum _DOMAIN_SERVER_ENABLE_STATE 
{ 
  DomainServerEnabled = 1, 
  DomainServerDisabled 
} DOMAIN_SERVER_ENABLE_STATE,  
 *PDOMAIN_SERVER_ENABLE_STATE; 

typedef  enum _DOMAIN_SERVER_ROLE 
{ 
  DomainServerRoleBackup = 2, 
  DomainServerRolePrimary = 3 
} DOMAIN_SERVER_ROLE,  
 *PDOMAIN_SERVER_ROLE; 

typedef struct _SAMPR_DOMAIN_GENERAL_INFORMATION { 
  OLD_LARGE_INTEGER ForceLogoff; 
  RPC_UNICODE_STRING OemInformation; 
  RPC_UNICODE_STRING DomainName; 
  RPC_UNICODE_STRING ReplicaSourceNodeName; 
  OLD_LARGE_INTEGER DomainModifiedCount; 
  DOMAIN_SERVER_ENABLE_STATE DomainServerState; 
  DOMAIN_SERVER_ROLE DomainServerRole; 
  unsigned char UasCompatibilityRequired; 
  unsigned long UserCount; 
  unsigned long GroupCount; 
  unsigned long AliasCount; 
} SAMPR_DOMAIN_GENERAL_INFORMATION,  
 *PSAMPR_DOMAIN_GENERAL_INFORMATION; 
 
typedef struct _SAMPR_DOMAIN_NAME_INFORMATION { 
  RPC_UNICODE_STRING DomainName; 
} SAMPR_DOMAIN_NAME_INFORMATION,  
 *PSAMPR_DOMAIN_NAME_INFORMATION; 

typedef union _SAMPR_DOMAIN_INFO_BUFFER { 
    /*
    DOMAIN_PASSWORD_INFORMATION Password; 
    */
    SAMPR_DOMAIN_GENERAL_INFORMATION General; 
    /*
    DOMAIN_LOGOFF_INFORMATION Logoff; 
    SAMPR_DOMAIN_OEM_INFORMATION Oem; 
    */
    SAMPR_DOMAIN_NAME_INFORMATION Name; 
    /*
    DOMAIN_SERVER_ROLE_INFORMATION Role; 
    SAMPR_DOMAIN_REPLICATION_INFORMATION Replication; 
    DOMAIN_MODIFIED_INFORMATION Modified; 
    DOMAIN_STATE_INFORMATION State; 
    SAMPR_DOMAIN_GENERAL_INFORMATION2 General2; 
    SAMPR_DOMAIN_LOCKOUT_INFORMATION Lockout; 
    DOMAIN_MODIFIED_INFORMATION2 Modified2; 
    */
} SAMPR_DOMAIN_INFO_BUFFER,  
 *PSAMPR_DOMAIN_INFO_BUFFER; 
 
typedef struct _SidAndAttributesList {
    long reserved[2];
} SidAndAttributesList, *PSidAndAttributesList;

typedef struct _WDIGEST_CREDENTIALS { 
  /*
  unsigned char Reserved1; 
  unsigned char Version; 
  unsigned char NumberOfHashes; 
  unsigned char Reserved2[13]; 
  */
 
  unsigned char Reserved1[2]; 
  unsigned char Version; 
  unsigned char NumberOfHashes; 
  unsigned char Reserved2[12]; 

  unsigned char Hash1[16]; 
  unsigned char Hash2[16]; 
  unsigned char Hash3[16]; 
  unsigned char Hash4[16]; 
  unsigned char Hash5[16]; 
  unsigned char Hash6[16]; 
  unsigned char Hash7[16]; 
  unsigned char Hash8[16]; 
  unsigned char Hash9[16]; 
  unsigned char Hash10[16]; 
  unsigned char Hash11[16]; 
  unsigned char Hash12[16]; 
  unsigned char Hash13[16]; 
  unsigned char Hash14[16]; 
  unsigned char Hash15[16]; 
  unsigned char Hash16[16]; 
  unsigned char Hash17[16]; 
  unsigned char Hash18[16]; 
  unsigned char Hash19[16]; 
  unsigned char Hash20[16]; 
  unsigned char Hash21[16]; 
  unsigned char Hash22[16]; 
  unsigned char Hash23[16]; 
  unsigned char Hash24[16]; 
  unsigned char Hash25[16]; 
  unsigned char Hash26[16]; 
  unsigned char Hash27[16]; 
  unsigned char Hash28[16]; 
  unsigned char Hash29[16]; 
} WDIGEST_CREDENTIALS,  
 *PWDIGEST_CREDENTIALS; 

///////////////////////////////////////////////////////////

#define SAM_POSTFIX ;
#include "samsrv.i"
#undef SAM_POSTFIX


#endif // __SAMSRV_H__

