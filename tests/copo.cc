
#include <iostream>
#include <sstream>
#include <vector>


#ifdef _WIN32
#include <windows.h>
 
 
// Converts UTF-16/wstring to UTF-8/string
std::string ws2s(const std::wstring ws) {
  if (ws.empty()) return std::string();

  UINT cp=CP_UTF8;
  DWORD flags=WC_ERR_INVALID_CHARS;
  auto wc=ws.c_str();
  auto wl=static_cast<int>(ws.size());
  auto l=WideCharToMultiByte(cp, flags, wc, wl, nullptr, 0, nullptr, nullptr);

  if (l > 0) {
    std::string s(static_cast<std::size_t>(l), '\0');
    if (WideCharToMultiByte(cp, flags, wc, wl, &s[0], l, nullptr, nullptr) > 0) return s;
  }

  return std::string();
}

// Converts UTF-8/string to UTF-16/wstring
std::wstring s2ws(const std::string s) {
  if (s.empty()) return std::wstring();

  UINT cp=CP_UTF8;
  DWORD flags=MB_ERR_INVALID_CHARS;
  auto c=s.c_str();
  auto l=static_cast<int>(s.size());
  auto wl=MultiByteToWideChar(cp, flags, c, l, nullptr, 0);
  if (wl > 0) {
    std::wstring ws(static_cast<std::size_t>(wl), L'\0');
    if (MultiByteToWideChar(cp, flags, c, l, &ws[0], wl) > 0) return ws;
  }

  return std::wstring();
}
#else
std::string ws2s(std::wstring ws)
{
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  return converter.to_bytes(ws);
}

std::wstring s2wss(std::string s)
{
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  return converter.from_bytes(s);
}
#endif

// Return false if wide char is a printable ascii else true
boolean not_printable_ascii(wchar_t wc) {
  if (wc > 31 && wc < 127) return false;
  return true;
}

// Convert non ascii characters of a wstring to html entities in decimal &#dec;
std::string htentd(const std::wstring ws) {
  std::stringstream ss;

  for (auto wc:ws) {
    if (not_printable_ascii(wc)) {
      ss << "&#" << (unsigned int)wc << ';';
    } else ss << (char)wc;
  }

  return ss.str();
}
 
// Same as previous for string
std::string htentd(const std::string s) {
  return htentd(s2ws(s));
}

// Convert non ascii characters of a wstring to html entities in decimal &#xhex;
std::string htenth(const std::wstring ws) {
  std::stringstream ss;

  for (auto wc:ws) {
    if (not_printable_ascii(wc)) ss << "&#x" << std::hex << (unsigned int)wc << ';';
    else ss << (char)wc;
  }
  return ss.str();
}
 
// Same as previous for string
std::string htenth(const std::string s) {
  return htenth(s2ws(s));
}

void out_cph(std::string s) {
  std::cout << s << std::endl << " ==> " << htenth(s2ws(s)) << std::endl;
}

void out_cph(std::wstring ws) {
  std::cout << ws2s(ws) << std::endl << " ==> " << htenth(ws) << std::endl;
}

void out_cp(std::string s) {
  std::cout << s << std::endl << " ==> " << htentd(s2ws(s)) << std::endl;
}

void out_cp(std::wstring ws) {
  std::cout << ws2s(ws) << std::endl << " ==> " << htentd(ws) << std::endl;
}

int main(int argc, char *argv[]) {
#ifdef _WIN32
  SetConsoleOutputCP(CP_UTF8);
#endif

  out_cph("पार्सल् एक्स्प्रेस्");
  out_cph("A你ètotö要らない你");
  out_cp(L"पार्सल् एक्स्प्रेस्");
  out_cp(L"A你ètotö要らない你");
  return 0;
}

