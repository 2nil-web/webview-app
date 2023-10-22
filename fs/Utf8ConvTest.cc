// Utf8ConvTest.cpp   -- from Giovanni Dicanio
// Unit test the UTF-8 <-> UTF-16 conversion functions declared in Utf8Conv.hpp
// You can also test the code undefining the following overflow check macro *before* including "Utf8Conv.hpp":
//#define GIOVANNI_DICANIO_UTF8CONV_CHECK_INTEGER_OVERFLOWS_ONLY_IN_DEBUG

#include <windows.h>
#include <tchar.h>
#include <iostream> 
#include <vector>


#ifdef LIST_CP
#include "code_page_list.h"
// EnumSystemCodePages callback function
BOOL CALLBACK EnumCodePagesProc(LPTSTR lpCodePageString) {
  int cp_id=_ttoi(lpCodePageString);
  for (auto cp:cp_info_list) {
    if (cp.id == cp_id) {
      std::cout << "id: " << cp.id << ", name: " << cp.name << ", desc " << cp.desc << std::endl;
      break;
    }
  }
  return TRUE;
}

void CPList() {
  EnumSystemCodePages(EnumCodePagesProc, CP_INSTALLED);
}
#else
void CPList() {}
#endif


#include "Utf8Conv.hpp"
using Utf8Conv::Utf16ToUtf8;
using Utf8Conv::Utf8ToUtf16;

std::string TestU8_to_U16(const std::string s1) {
  std::wstring ws = Utf8ToUtf16(s1);
  std::string s2 = Utf16ToUtf8(ws);
  std::string res=(s1 == s2)?"OK":"Failed";
  return std::string("UTF8>16>8   loop is ")+res+" for '"+s1+"'\n";
}

std::string TestU16_to_U8(const std::wstring ws1) {
  std::string s = Utf16ToUtf8(ws1);
  std::wstring ws2 = Utf8ToUtf16(s);
  std::string res=(ws1 == ws2)?"OK":"Failed";
  return "UTF16>8>16 loop is "+res+" for '"+s+"'\n";
}

std::wstring wTestU8_to_U16(const std::string s1) {
  std::wstring ws = Utf8ToUtf16(s1);
  std::string s2 = Utf16ToUtf8(ws);
  std::wstring res=(s1 == s2)?L"OK":L"Failed";
  return L"UTF8>16>8   loop is "+res+L" for '"+ws+L"'\n";
}

std::wstring wTestU16_to_U8(const std::wstring ws1) {
  std::string s = Utf16ToUtf8(ws1);
  std::wstring ws2 = Utf8ToUtf16(s);
  std::wstring res=(ws1 == ws2)?L"OK":L"Failed";
  return L"UTF16>8>16 loop is "+res+L" for '"+ws1+L"'\n";
}

int MessageBox(const std::string caption, const std::string text, UINT type=MB_OK) {
  return MessageBoxA(nullptr, text.c_str(), caption.c_str(), type);
}

int MessageBox(const std::wstring caption, const std::wstring text, UINT type=MB_OK) {
  return MessageBoxW(nullptr, text.c_str(), caption.c_str(), type);
}

int main() {
  SetConsoleOutputCP(CP_UTF8);
  CPList();

  std::cout << "Running tests..." << std::endl;

  std::wstring msg=L"";

  msg+=wTestU8_to_U16("abcdEF");
  msg+=wTestU8_to_U16("_éèàôû_");
  msg+=wTestU8_to_U16("_要らない_");

  msg+=L'\n';
  msg+=wTestU16_to_U8(L"abcdEF");
  msg+=wTestU16_to_U8(L"_éèàôû_");
  msg+=wTestU16_to_U8(L"_要らない_");

  std::string amsg="";
  amsg+=TestU16_to_U8(L"abcdEF");
  amsg+=TestU16_to_U8(L"_éèàôû_");
  amsg+=TestU16_to_U8(L"_要らない_");

  std::cout << amsg;

  MessageBox(L"UTF tests", msg);
  return 0;
}

