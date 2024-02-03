

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <windows.h>

template <class T> std::string Ts2h(T s)
{
  std::stringstream cnv;
  std::string res = "";

  bool one = true;
  for (auto c : s)
  {
    unsigned int ui = (unsigned int)c;
    if (one)
    {
      std::cout << "sizeof(c)  " << sizeof(c) << std::endl;
      std::cout << "sizeof(ui) " << sizeof(ui) << std::endl;
      one = false;
    }
    if (ui > 32 && ui < 127)
    {
      res += (char)c;
    }
    else
    {
      cnv.str("");
      cnv << ' ' << std::hex << ui;
      res += cnv.str();
    }
  }

  return res;
}

int main(int, char *[])
{
  SetConsoleOutputCP(CP_UTF8);
  std::cout << "sizeof(std::wstring) " << sizeof(std::wstring) << std::endl;
  std::cout << "sizeof(std::string)  " << sizeof(std::string) << std::endl;
  std::wstring ws = L"string=😀ドイツ語で検索していてこちらのサイトにたどり着きました。";
  std::cout << Ts2h(ws) << std::endl;

  std::string s = "string=😀ドイツ語で検索していてこちらのサイトにたどり着きました。";
  std::cout << Ts2h(s) << std::endl;

  return 0;
}
