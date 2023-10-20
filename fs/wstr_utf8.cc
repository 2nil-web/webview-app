
#include <string>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <windows.h>


// Convert wstring (UTF16) to string (UTF8) in Window context
std::string ws2s(const std::wstring s) {
  std::string ret;
  int cbn=WideCharToMultiByte(CP_UTF8, 0, s.c_str(), -1, NULL, 0, NULL, NULL);

  if (cbn > 0) {
    char *utf8=new char[cbn];
    if (WideCharToMultiByte(CP_UTF8, 0, s.c_str(), -1, utf8, cbn, NULL, NULL) != 0) ret=utf8;
    delete[] utf8;
  }

  return ret;
}

// Convert string (UTF8) to wstring (UTF16) in Window context
std::wstring s2ws(const std::string s) {
  std::wstring ret;
  int cchn=MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, NULL, 0);

  if (cchn > 0) {
    wchar_t *utf16=new wchar_t[cchn];
    if (MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, utf16, cchn) != 0) ret=utf16;
    delete[] utf16;
  }

  return ret;
}

// Convert an utf8 string into an url encoded hexadecimal one
std::string s2h(const std::string s) {
  std::string ret;
  char hs[5];
  unsigned char uc;
  for (const char *p=s.c_str(); *p; p++) {
    uc=(unsigned char)*p;
    _snprintf(hs, sizeof(hs), "%%%2.2X", uc);
    ret.append(hs);
  }

  return ret;
}

// Convert an url hexadecimal encoded string into an utf8 string 
std::string h2s(const std::string s) {
  std::string ret;
  int ch;

  for (const char *p=s.c_str(); *p; ) {
    if (p[0] == '%' && isxdigit(p[1]) && isxdigit(p[2])) {
      ch=(isdigit(p[1]) ? p[1] - '0' : toupper(p[1]) - 'A' + 10) * 16 + (isdigit(p[2]) ? p[2] - '0' : toupper(p[2]) - 'A' + 10);
      ret.push_back((char)ch);
      p += 3;
    } else if (p[0] == '%' && p[1] == '#' && isdigit(p[2])) {
      ch=atoi(p + 2);
      ret.push_back((char)ch);
      p += 2;
      while (*p && isdigit(*p)) p++;
      if (*p == ';') p++;
    } else ret.push_back(*p++);
  }

  return ret;
}

// Convert a wstring into an url encoded hexadecimal string
std::string ws2u8h(const std::wstring s) {
  return s2h(ws2s(s));
}

// Convert an url encoded hexadecimal string into a wstring
std::wstring u8h2ws(const std::string s) {
  return s2ws(h2s(s));
}

// Convert an utf8 string into an url encoded hexadecimal one
std::string EX_s2h(const std::string s) {
  std::string ret;
  char hs[5];
  unsigned char uc;
  for (const char *p=s.c_str(); *p; p++) {
    uc=(unsigned char)*p;
    _snprintf(hs, sizeof(hs), "%%%2.2X", uc);
    ret.append(hs);
  }

  return ret;
}

void prth(std::wstring ws) {
  size_t i=0;
  for (auto wc : ws) {
    unsigned int ui=(unsigned int)wc;
    std::cout << ws2s(ws.substr(i++, 1));
    std::cout << " # ";
    std::cout << std::hex << ui;
    std::cout << " # ";
    std::cout << std::dec << ui;
    std::cout << std::endl;
  }

  std::cout << ws2s(ws) << std::endl;
}

void prth(std::string s) {
  prth(s2ws(s));
}

std::string s2he(std::string s) {

}

// Convert non ascii portion of a wstring into url encoded hexadecimal string
std::string url_encode(std::wstring ws){
  std::string s=ws2s(ws);
  unsigned int uc;
  char hs[5];

  for (const char *p=s.c_str(); *p; p++) {
    uc=(unsigned char)*p;
    if (uc < 128) s += *p;
    else {
      _snprintf(hs, sizeof(hs), "%%%02.2X", uc);
      //if (uc < 16) s += "%0"; 
      //else s += "%";
      s.append(hs);
    }
  }

  return s;
 }

// Convert string containing url encoded hexadecimal value into a wstring 
std::wstring url_decode(std::string str) {
  std::string ret;
  char ch;
  int i, ii, len=str.length();

  for (i=0; i < len; i++) {
    if (str[i] != '%') {
      if (str[i] == '+') ret += ' ';
      else ret += str[i];
    } else {
      sscanf(str.substr(i + 1, 2).c_str(), "%x", &ii);
      ch=static_cast<char>(ii);
      ret += ch;
      i=i + 2;
    }
  }

  return s2ws(ret);
}

// https://www.w3schools.com/charsets/ref_html_utf8.asp
int main(int, char*[]) {
   SetConsoleOutputCP(CP_UTF8);
/*
 
  prth(L"string=®😀ドイツ語で検索していてこちらのサイトにたどり着きました。");
  return 0;
  std::cout << "wstring to utf8 hex" << std::endl;
  std::wstring ws_ja=L"string=ドイツ語で検索していてこちらのサイトにたどり着きました。";
  std::string s_ja=url_encode(ws_ja);
  std::wstring ws_ja2=url_decode(s_ja);
  std::cout << "from=" << ws2s(ws_ja) << std::endl << std::flush;
  std::cout << "to=" << s_ja << std::endl << std::flush;
  std::cout << "back=" << ws2s(ws_ja2) << std::endl << std::flush;

*/

  std::string sja="string=😀ドイツ語で検索していてこちらのサイトにたどり着きました。";
  prth(sja);

  std::wstring ja=L"string=😀ドイツ語で検索していてこちらのサイトにたどり着きました。";
  prth(ja);
  return 0;
  std::cout << "wstring to utf8 hex " << std::endl;
  std::cout << ws2s(ja) << std::endl;
  std::cout << ws2u8h(ja) << std::endl;

  std::cout << "utf8 hex to wstring " << std::endl;
  std::string hex="%E3%83%89%E3%82%A4%E3%83%84%E8%AA%9E%E3%81%A7%E6%A4%9C%E7%B4%A2%E3%81%97%E3%81%A6%E3%81%84%E3%81%A6%E3%81%93%E3%81%A1%E3%82%89%E3%81%AE%E3%82%B5%E3%82%A4%E3%83%88%E3%81%AB%E3%81%9F%E3%81%A9%E3%82%8A%E7%9D%80%E3%81%8D%E3%81%BE%E3%81%97%E3%81%9F%E3%80%82";
  std::cout << hex << std::endl;
  std::wstring newja=u8h2ws(hex);
  std::cout << ws2s(newja) << std::endl;
  std::cout << "match?=" << (newja == ja ? "yes" : "no") << std::endl;

  return 0;
}

