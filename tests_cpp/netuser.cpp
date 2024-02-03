
#ifdef _WIN32
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
#include <sddl.h>
#else
#include <string.h>
#include <unistd.h>
#endif

#include <algorithm>
#include <chrono>
#include <codecvt>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <locale>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || __cplusplus >= 201703L)
#include <filesystem>
#endif

#ifdef _MSC_VER
// Converts UTF-16/wstring to UTF-8/string
std::string ws2s(const std::wstring ws)
{
  if (ws.empty())
    return std::string();

  UINT cp = CP_UTF8;
  DWORD flags = WC_ERR_INVALID_CHARS;
  auto wc = ws.c_str();
  auto wl = static_cast<int>(ws.size());
  auto l = WideCharToMultiByte(cp, flags, wc, wl, nullptr, 0, nullptr, nullptr);

  if (l > 0)
  {
    std::string s(static_cast<std::size_t>(l), '\0');
    if (WideCharToMultiByte(cp, flags, wc, wl, &s[0], l, nullptr, nullptr) > 0)
      return s;
  }

  return std::string();
}

// Converts UTF-8/string to UTF-16/wstring
std::wstring s2ws(const std::string s)
{
  if (s.empty())
    return std::wstring();

  UINT cp = CP_UTF8;
  DWORD flags = MB_ERR_INVALID_CHARS;
  auto c = s.c_str();
  auto l = static_cast<int>(s.size());
  auto wl = MultiByteToWideChar(cp, flags, c, l, nullptr, 0);
  if (wl > 0)
  {
    std::wstring ws(static_cast<std::size_t>(wl), L'\0');
    if (MultiByteToWideChar(cp, flags, c, l, &ws[0], wl) > 0)
      return ws;
  }

  return std::wstring();
}
#else
std::string ws2s(std::wstring ws)
{
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  return converter.to_bytes(ws);
}

std::wstring s2ws(std::string s)
{
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  return converter.from_bytes(s);
}
#endif


void tab_print_header(std::string sep=";") {
  std::cout << "Name" << sep <<
   "Password" << sep <<
   "Password age" << sep << "Priv" << sep << "Home dir" << sep <<
   "Comment" << sep <<
   "Flags" << sep << "Script path" << sep << "Auth flags" << sep << "Full name" << sep << "Usr comment" << sep <<
   "Parms" << sep << "Workstations" << sep << "Last logon" << sep << "Last logoff" << sep << "Acct expires" << sep <<
   "Max storage" << sep << "Units per week" << sep << "Logon hours" << sep << "Bad pw count" << sep << "Num logons" << sep <<
   "Logon server" << sep << "Country code" << sep << "Code page" << sep << "User id" << sep << "Primary group id" << sep <<
   "Profile" << sep << "Home dir drive" << sep << "Password expired" << std::endl;
}

// Convert non ascii characters of a wstring to html entities form in decimal (default) or hexa &#[x]value;
std::string to_htent(const wchar_t* pwc, bool dec_base=true)
{
  if (pwc) {
    std::stringstream ss;
    std::wstring ws=std::wstring(pwc);

    for (auto wc : ws)
    {
      if (!isascii(wc))
      {
        ss << "&#";
        if (dec_base)
          ss << std::dec;
        else
          ss << 'x' << std::hex;
        ss << (unsigned int)wc << ';';
      }
      else
        ss << (char)wc;
    }

    return ss.str();
  }

  return "";
}

void tab_print_usri3 (LPUSER_INFO_3 u, std::string sep=";") {
  std::cout <<
    to_htent(u->usri3_name) << sep << to_htent(u->usri3_password) << sep << u->usri3_password_age << sep <<
    u->usri3_priv << sep << to_htent(u->usri3_home_dir) << sep << to_htent(u->usri3_comment) << sep <<
    u->usri3_flags << sep << to_htent(u->usri3_script_path) << sep << u->usri3_auth_flags << sep <<
    to_htent(u->usri3_full_name) << sep << to_htent(u->usri3_usr_comment) << sep << to_htent(u->usri3_parms) << sep <<
    to_htent(u->usri3_workstations) << sep << u->usri3_last_logon << sep << u->usri3_last_logoff << sep <<
    u->usri3_acct_expires << sep << u->usri3_max_storage << sep <<
    u->usri3_units_per_week << sep;

    for(int i=0; i < 21; i++) { std::cout << std::hex << (BYTE) u->usri3_logon_hours[i]; }
    std::cout << std::dec << sep <<

    u->usri3_bad_pw_count << sep << u->usri3_num_logons << sep << to_htent(u->usri3_logon_server) << sep <<
    u->usri3_country_code << sep << u->usri3_code_page << sep << u->usri3_user_id << sep <<
    u->usri3_primary_group_id << sep << to_htent(u->usri3_profile) << sep << to_htent(u->usri3_home_dir_drive) << sep <<
    u->usri3_password_expired << sep <<
    std::endl;
}


void form_print_usri3 (LPUSER_INFO_3 u) {
  std::cout << "Name:" << to_htent(u->usri3_name) << std::endl;
  std::cout << "Password:" << to_htent(u->usri3_password) << std::endl;
  std::cout << "Password_age:" << u->usri3_password_age << std::endl;
  std::cout << "Priv:" << u->usri3_priv << std::endl;
  std::cout << "Home dir:" << to_htent(u->usri3_home_dir) << std::endl;
  std::cout << "Comment:" << to_htent(u->usri3_comment) << std::endl;
  std::cout << "Flags:" << u->usri3_flags << std::endl;
  std::cout << "Script path:" << to_htent(u->usri3_script_path) << std::endl;
  std::cout << "Auth flags:" << u->usri3_auth_flags << std::endl;
  std::cout << "Full name:" << to_htent(u->usri3_full_name) << std::endl;
  std::cout << "Usr comment:" << to_htent(u->usri3_usr_comment) << std::endl;
  std::cout << "Parms:" << to_htent(u->usri3_parms) << std::endl;
  std::cout << "Workstations:" << to_htent(u->usri3_workstations) << std::endl;
  std::cout << "Last logon:" << u->usri3_last_logon << std::endl;
  std::cout << "Last logoff:" << u->usri3_last_logoff << std::endl;
  std::cout << "Acct expires:" << u->usri3_acct_expires << std::endl;
  std::cout << "Max storage:" << u->usri3_max_storage << std::endl;
  std::cout << "Units per week:" << u->usri3_units_per_week << std::endl;

  std::cout << "Logon hours:";
  for (int i = 0; i < 21; i++) { std::cout << std::hex << (BYTE) u->usri3_logon_hours[i]; }
  std::cout << std::dec << std::endl;

  std::cout << "Bad pw count:" << u->usri3_bad_pw_count << std::endl;
  std::cout << "Num logons:" << u->usri3_num_logons << std::endl;
  std::cout << "Logon server:" << to_htent(u->usri3_logon_server) << std::endl;
  std::cout << "Country code:" << u->usri3_country_code << std::endl;
  std::cout << "Code page:" << u->usri3_code_page << std::endl;
  std::cout << "User id:" << u->usri3_user_id << std::endl;
  std::cout << "Primary group_id:" << u->usri3_primary_group_id << std::endl;
  std::cout << "Profile:" << to_htent(u->usri3_profile) << std::endl;
  std::cout << "Home dir drive:" << to_htent(u->usri3_home_dir_drive) << std::endl;
  std::cout << "Password expired:" << u->usri3_password_expired << std::endl;
  std::cout << std::endl;
}

void NetUserGetInfoForm(LPCWSTR serverName, LPCWSTR userName) {
  LPUSER_INFO_0 pBuf = NULL;
  NET_API_STATUS nStatus;

  //std::wcout << L"\nCalling NetUserGetinfo with Servername=" << serverName << ", Username=" << userName << ", Level=3" << std::endl;
  nStatus = NetUserGetInfo(serverName, userName, 3, (LPBYTE *) & pBuf);

  // If the call succeeds, print the user information.
  if (nStatus == NERR_Success) {
    if (pBuf != NULL) form_print_usri3((LPUSER_INFO_3) pBuf);
  } else fprintf(stderr, "NetUserGetinfo failed with error: %ld\n", nStatus);

  // Free the allocated memory.
  if (pBuf != NULL) NetApiBufferFree(pBuf);
}

void NetQueryDisplayInformationList(LPCWSTR serverName, std::string sep=";") {
   PNET_DISPLAY_USER  pBuff, p;
   DWORD res, dwRec, i = 0;

   std::cout << "Name" << sep << "Comment" << sep << "Flags" << sep << "Full name" << sep << "User id" << std::endl;

   do {
      // Call the NetQueryDisplayInformation function; specify information level 1 (user account information).
      res = NetQueryDisplayInformation(serverName, 1, i, 1000, MAX_PREFERRED_LENGTH, &dwRec, (PVOID*) &pBuff);

      // If the call succeeds,
      if ((res==ERROR_SUCCESS) || (res==ERROR_MORE_DATA)) {
         p = pBuff;
         for(;dwRec>0;dwRec--) {
           std::cout <<  to_htent(p->usri1_name); std::cout << sep;
           std::cout <<  to_htent(p->usri1_comment); std::cout << sep;
           std::cout <<  p->usri1_flags; std::cout << sep;
           std::cout <<  to_htent(p->usri1_full_name); std::cout << sep;
           std::cout <<  p->usri1_user_id; std::cout << sep;
           std::cout <<  p->usri1_next_index;
           std::cout << std::endl;

           // If there is more data, set the index.
           i = p->usri1_next_index;
           p++;
         }
         // Free the allocated memory.
         NetApiBufferFree(pBuff);
      } else std::cerr << "Error:" << res << std::endl;
   // Continue while there is more data.
   } while (res==ERROR_MORE_DATA); // end do
   return;
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
      NetQueryDisplayInformationList(NULL);
      //NetUserEnumList(NULL);
      break;
    case 2:
      // Only server provided so list all users for it.
      std::wcout << "\nUser(s) account on " << argv[1] << std::endl;
      NetQueryDisplayInformationList(argv[1]);
      //NetUserEnumList(argv[1]);
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

