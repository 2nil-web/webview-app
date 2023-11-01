
#include <iostream>
#include <sstream>
#include <vector>
#include <windows.h>
 
 
int codepoint(const std::string u) {
  auto l = u.length();
  if (l < 1) return -1;

  unsigned char u0 = u[0];
  if (u0>=0   && u0<=127) return u0;

  if (l < 2) return -1;

  unsigned char u1 = u[1];
  if (u0>=192 && u0<=223) return (u0-192)*64 + (u1-128);

  if (u[0]==0xed && (u[1] & 0xa0) == 0xa0) return -1;

  if (l < 3) return -1;

  unsigned char u2 = u[2];
  if (u0>=224 && u0<=239) return (u0-224)*4096 + (u1-128)*64 + (u2-128);

  if (l<4) return -1;

  unsigned char u3 = u[3];
  if (u0>=240 && u0<=247) return (u0-240)*262144 + (u1-128)*4096 + (u2-128)*64 + (u3-128);

  return -1;
}
 
std::string codepointhex(const std::string u) {
  std::stringstream ss;
  std::string s,cs;
  for (auto c:u) {
    cs=c;
    ss << std::showbase << "&#" << std::hex << cs << ';';
  }
  ss >> s;
  return s;
}
 
void out_cp(std::string s) {
  std::cout << s << ':' << codepointhex(s) << std::endl;
}

std::wstring codepointhex(const std::wstring u) {
  std::wstringstream ss;
  std::wstring s,cs;
  for (auto c:u) {
    ss << std::showbase << "&#" << std::hex << (int)c << ';';
  }
  ss >> s;
  return s;
}
 
void out_cp(std::wstring s) {
  std::wcout << codepointhex(s) << std::endl;
}

std::string utf8chr(int cp) {
  char c[5]={ 0x00,0x00,0x00,0x00,0x00 };

  if      (cp < 0x80)     { c[0]=cp;  }
  else if (cp < 0x800)    { c[0]=(cp>>6)+0xc0; c[1]=(cp&0x3f)+0x80; }
  else if (cp > 0xd801 && cp < 0xe000) {} //invalid block of utf8
  else if (cp < 0x10000)  { c[0]=(cp>>0xc )+0xe0; c[1]=((cp>>6)&0x3f  )+0x80; c[2]= (cp&0x3f)+0x80; }
  else if (cp < 0x110000) { c[0]=(cp>>0x12)+0xf0; c[1]=((cp>>0xC)&0x3f)+0x80; c[2]=((cp>>6)&0x3f)+0x80; c[3]=(cp&0x3f)+0x80; }
  return std::string(c);
}

int main(int argc, char *argv[]) {
  SetConsoleOutputCP(CP_UTF8);
#ifdef NON
  std::vector<std::string> args(argv, argv + argc);
  if (args.size() < 2) {
    std::cout << codepointhex("###_要らない_###") << std::endl;
    std::cout << codepointhex("###_éèàôû_###") << std::endl;
  } else for(auto arg:args) std::cout << codepointhex(arg) << std::endl;
#else
#ifdef NONON
    //for (int i= 0; i < 2048; i++) std::cout << utf8chr(i);
    for (int i= 0; i < 2048; i++) std::cout << "&#" << std::hex << i <<';';
    std::cout << std::endl;
    //printable ascii range
    for (int i=0x20;  i < 0x7f;   i++) { std::cout << i << ":" << utf8chr(i) << std::endl; }
    // À to ž
    for (int i=0xc0;  i < 0x17e;  i++) { std::cout << i << ":" << utf8chr(i) << std::endl; }
    // 你 to 使
    for(int i=0x4f60; i < 0x4f80; i++) { std::cout << i << ":" << utf8chr(i) << std::endl; }
#else
 
    out_cp(L"A你ètotö要らない你");
    out_cp("A你ètotö要らない你");
    out_cp(L"पार्सल् एक्स्प्रेस्");
    //out_cp("A"); out_cp("你"); out_cp("è"); out_cp("t"); out_cp("o"); out_cp("t"); out_cp("ö"); out_cp("要"); out_cp("ら"); out_cp("な"); out_cp("い"); out_cp("你"); 
    //out_cp("पा"); out_cp("र्"); out_cp("स"); out_cp("ल्"); out_cp(" "); out_cp("ए"); out_cp("क्"); out_cp("स्"); out_cp("प्"); out_cp("रे"); out_cp("स्"); 

//    std::cout << input0 << "," << codepoint(input0) << "," << codepointhex(input0) << std::endl; //65,0x41
//    std::cout << input1 << "," << codepoint(input1) << "," << codepointhex(input1) << std::endl; //232,0xe8
//    std::cout << input2 << "," << codepoint(input2) << "," << codepointhex(input2) << std::endl; //20320,0x4f60
#endif
#endif
    return 0;
}

