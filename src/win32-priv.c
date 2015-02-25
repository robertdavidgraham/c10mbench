#ifdef WIN32
#include <Windows.h>
#include <stdio.h>
#include <ntsecapi.h>
#include <Sddl.h> // for ConvertSidToStringSid()

static void win_perror(const char *str)
{
    DWORD dwError = GetLastError();
    LPVOID lpvMessageBuffer;

    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                  FORMAT_MESSAGE_FROM_SYSTEM |
                  FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL, dwError,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPSTR)&lpvMessageBuffer, 0, NULL);

    //... now display this string
    fprintf(stderr, "%s: %s\n", str, lpvMessageBuffer);

    
    // Free the buffer allocated by the system
    LocalFree(lpvMessageBuffer);

}

void foo(void)
{
    TOKEN_USER *user_token = 0;
    DWORD sizeof_user_token = 0;
    DWORD x;
    HANDLE h;
    LSA_HANDLE policy_handle;

    

    /*
     * Get the current process handle
     */
    x = OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &h);
    if (!x) {
        win_perror("OpenProcessToken");
        exit(1);
    }


    /*
     * Get the token of the current user
     */
    again:
    x = GetTokenInformation(h,
                            TokenUser, 
                            user_token, 
                            sizeof_user_token, 
                            &sizeof_user_token);

    if (x == 0 && GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
        user_token = (TOKEN_USER*)malloc(sizeof_user_token);
        if (user_token) {
            memset(user_token, 0, sizeof_user_token);
            goto again;
        }
    }
    if (x == 0) {
        win_perror("GetTokenInformation");
        exit(1);
    }
    CloseHandle(h);

    {
        char *StringSid = 0;
        ConvertSidToStringSidA(user_token->User.Sid, &StringSid);
        printf("%s\n", StringSid);
    }

    {
        DWORD dwLength = GetLengthSid(user_token->User.Sid);
        printf("%u\n", dwLength);
    }

    /*
     * 
     */
    {
        NTSTATUS n;
        LSA_OBJECT_ATTRIBUTES attr;
        
        memset(&attr, 0, sizeof(attr));

        n = LsaOpenPolicy(NULL, &attr, 
                POLICY_VIEW_LOCAL_INFORMATION|POLICY_LOOKUP_NAMES, 
                &policy_handle);
        if (n) {
            SetLastError(LsaNtStatusToWinError(n));
            win_perror("LsaOpenPolicy");
            exit(1);
        }
    }

    {
        NTSTATUS n;
        LSA_UNICODE_STRING *UserRights = 0;
        ULONG CountOfRights = 0;

        n = LsaEnumerateAccountRights(
            policy_handle, 
            user_token->User.Sid,
            &UserRights,
            &CountOfRights);
        if (n) {
            SetLastError(LsaNtStatusToWinError(n));
            win_perror("LsaEnumerateAccountRights");
            exit(1);
        }
    }



    return;
}

#if 0
static int
ProcessTokenToSid(HANDLE token, int *SID)
{
    TOKEN_USER ;
    const int bufLength = 256;            
    int *tu = (int*)malloc(bufLength * sizeof(int));
    int ret = 0;
    DWORD cb = bufLength;

    SID =0;
    ret = GetTokenInformation(
                token, 
                1 /*TokenUser*/, 
                tu, cb, &cb);
    if (ret) {
        tokUser = (TOKEN_USER)Marshal.PtrToStructure(tu, typeof(TOKEN_USER));
        SID = tokUser.User.Sid;
    }
    return ret;
}


static int
DumpUserInfo(HANDLE pToken, int *SID)
{
    int Access = ;
    HANDLE procToken = NULL;
    int ret = 0;
    SID = NULL;

    ret = OpenProcessToken(pToken, TOKEN_QUERY, &procToken);
    if (ret) {
        ret = ProcessTokenToSid(procToken, SID);
        CloseHandle(procToken);
    }
    return ret;
}

public static string ExGetProcessInfoByPID
    (int PID, out string SID)//, out string OwnerSID)
{                                                                  
    IntPtr _SID = IntPtr.Zero;                                       
    SID = String.Empty;                                             
    try                                                             
    {                                                                
        Process process = Process.GetProcessById(PID);
        if (DumpUserInfo(process.Handle, out _SID))
        {                                                                    
            ConvertSidToStringSid(_SID, ref SID);
        }
        return process.ProcessName;                                          
    }                                                                           
    catch
    {                                                                           
        return "Unknown";
    }
}
#endif
#endif

