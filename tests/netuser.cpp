
#ifndef UNICODE
#define UNICODE
#endif

#pragma comment(lib, "netapi32.lib")

#include <iostream>
#include <stdio.h>
#include <assert.h>
#include <windows.h> 
#include <shellapi.h>
#include <lm.h>

void tab_print_header(std::string sep=";") {
  std::cout << "Name" << sep << 
   // "Password" << sep << 
   "Password_age" << sep << "Priv" << sep << "Home_dir" << sep <<
   "Comment" << sep << "Flags" << sep << "Script_path" << sep << "Auth_flags" << sep << "Full_name" << sep << "Usr_comment" << sep <<
   "Parms" << sep << "Workstations" << sep << "Last_logon" << sep << "Last_logoff" << sep << "Acct_expires" << sep <<
   "Max_storage" << sep << "Units_per_week" << sep << "Logon_hours" << sep << "Bad_pw_count" << sep << "Num_logons" << sep <<
   "Logon_server" << sep << "Country_code" << sep << "Code_page" << sep << "User_id" << sep << "Primary_group_id" << sep <<
   "Profile" << sep << "Home_dir_drive" << sep << "Password_expired" << std::endl;
}

void tab_print_usri3 (LPUSER_INFO_3 u, std::wstring sep=L"];[") {
  std::wcout << '[' <<
    u->usri3_name << sep <<
    //u->usri3_password << sep << 
    u->usri3_password_age << sep << u->usri3_priv << sep <<
    u->usri3_home_dir << sep << u->usri3_comment << sep << u->usri3_flags << sep << u->usri3_script_path << sep << u->usri3_auth_flags << sep <<
    u->usri3_full_name << sep << u->usri3_usr_comment << sep << u->usri3_parms << sep << u->usri3_workstations << sep <<
    u->usri3_last_logon << sep << u->usri3_last_logoff << sep << u->usri3_acct_expires << sep << u->usri3_max_storage << sep <<
    u->usri3_units_per_week << sep << u->usri3_logon_hours << sep << u->usri3_bad_pw_count << sep << u->usri3_num_logons << sep <<
    u->usri3_logon_server << sep << u->usri3_country_code << sep << u->usri3_code_page << sep << u->usri3_user_id << sep <<
    u->usri3_primary_group_id << sep << u->usri3_profile << sep << u->usri3_home_dir_drive << sep << u->usri3_password_expired << sep <<
    ']' << std::endl;
}

void form_print_usri3 (LPUSER_INFO_3 u) {
  std::wcout << "Name:" << u->usri3_name << std::endl;
  std::wcout << "Password:" << u->usri3_password << std::endl;
  std::wcout << "Password_age:" << u->usri3_password_age << std::endl;
  std::wcout << "Priv:" << u->usri3_priv << std::endl;
  std::wcout << "Home_dir:" << u->usri3_home_dir << std::endl;
  std::wcout << "Comment:" << u->usri3_comment << std::endl;
  std::wcout << "Flags:" << u->usri3_flags << std::endl;
  std::wcout << "Script_path:" << u->usri3_script_path << std::endl;
  std::wcout << "Auth_flags:" << u->usri3_auth_flags << std::endl;
  std::wcout << "Full_name:" << u->usri3_full_name << std::endl;
  std::wcout << "Usr_comment:" << u->usri3_usr_comment << std::endl;
  std::wcout << "Parms:" << u->usri3_parms << std::endl;
  std::wcout << "Workstations:" << u->usri3_workstations << std::endl;
  std::wcout << "Last_logon:" << u->usri3_last_logon << std::endl;
  std::wcout << "Last_logoff:" << u->usri3_last_logoff << std::endl;
  std::wcout << "Acct_expires:" << u->usri3_acct_expires << std::endl;
  std::wcout << "Max_storage:" << u->usri3_max_storage << std::endl;
  std::wcout << "Units_per_week:" << u->usri3_units_per_week << std::endl;
  std::wcout << "Logon_hours:" << u->usri3_logon_hours << std::endl;
  std::wcout << "Bad_pw_count:" << u->usri3_bad_pw_count << std::endl;
  std::wcout << "Num_logons:" << u->usri3_num_logons << std::endl;
  std::wcout << "Logon_server:" << u->usri3_logon_server << std::endl;
  std::wcout << "Country_code:" << u->usri3_country_code << std::endl;
  std::wcout << "Code_page:" << u->usri3_code_page << std::endl;
  std::wcout << "User_id:" << u->usri3_user_id << std::endl;
  std::wcout << "Primary_group_id:" << u->usri3_primary_group_id << std::endl;
  std::wcout << "Profile:" << u->usri3_profile << std::endl;
  std::wcout << "Home_dir_drive:" << u->usri3_home_dir_drive << std::endl;
  std::wcout << "Password_expired:" << u->usri3_password_expired << std::endl;
  std::cout << std::endl;
}

void NetUserGetInfoForm(LPCWSTR serverName, LPCWSTR userName) {
//  NetUserGetInfo(serverName, userName);
}

void NetUserEnumList(LPCWSTR serverName) {
  LPUSER_INFO_3 pBuf=NULL;
  LPUSER_INFO_3 pTmpBuf;
  DWORD dwLevel=3;
  DWORD dwPrefMaxLen=MAX_PREFERRED_LENGTH;
  DWORD dwEntriesRead=0;
  DWORD dwTotalEntries=0;
  DWORD dwResumeHandle=0;
  DWORD i;
  DWORD dwTotalCount=0;
  NET_API_STATUS nStatus;

  tab_print_header();
  // Call the NetUserEnum function, specifying level 0 to  enumerate global user account types only.
  do {
    nStatus=NetUserEnum(serverName, dwLevel, FILTER_NORMAL_ACCOUNT, (LPBYTE*)&pBuf, dwPrefMaxLen, &dwEntriesRead, &dwTotalEntries, &dwResumeHandle);
 
    // If the call succeeds,
    if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA))
    {
      if ((pTmpBuf=pBuf) != NULL)
      {
        // Loop through the entries.
        for (i=0; i < dwEntriesRead; i++)
        {
          assert(pTmpBuf != NULL);

          if (pTmpBuf == NULL)
          {
            std::cerr << "An access violation has occurred" << std::endl;
            break;
          }

          // Print the name of the user account.
          tab_print_usri3(pTmpBuf);
          //std::wcout << pTmpBuf->usri0_name << std::endl;
          pTmpBuf++;
          dwTotalCount++;
        }
      }
    } else {
      // Otherwise, print the system error.
      std::cerr << "A system error has occurred: " << nStatus << std::endl;
    }
 
    // Free the allocated buffer.
    if (pBuf != NULL)
    {
      NetApiBufferFree(pBuf);
      pBuf=NULL;
    }
  } while (nStatus == ERROR_MORE_DATA); // Continue to call NetUserEnum while there are more entries. 

  // Double checking to free eventually remaining allocated memory.
  if (pBuf != NULL) {
    NetApiBufferFree(pBuf);
  }

  // Print the final count of users enumerated.
  std::cerr << "\nTotal of " << dwTotalCount << " entries enumerated" << std::endl;
}

int main()//int argc, wchar_t *argv[])
{
  int argc;
  LPWSTR* argv=CommandLineToArgvW(GetCommandLineW(), &argc);
  //std::cout << argc << " arguments" << std::endl; for (int i=0; i < argc; i++) std::wcout << argv[i] << std::endl; return 0;

  switch (argc) {
    case 1:
      // No server nor user indicated then list of all users of the local machine.
      std::cout << "\nUser(s) account on local machine." << std::endl;
      NetUserEnumList(NULL);
      break;
    case 2:
      // Only server provided so list all users for it.
      std::wcout << "\nUser(s) account on " << argv[1] << std::endl;
      NetUserEnumList(argv[1]);
      break;
    case 3:
      // User and server provided then get detailled info for the user.
      std::wcout << "\nDetails of user " << argv[1] << " on server " << argv[2] << std::endl;
      NetUserGetInfoForm(argv[1], argv[2]);
      break;
    default :
      std::wcerr <<  "Usage: " << argv[0] << " [server name] [user name], no more than 2 arguments." << std::endl;
      return 1;
  }

  return 0;
}

