
#include <cstdio>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#else
#include <clocale>
#include <locale>
#endif

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <thread>

// Convert a wstring to a string containing a suite of hexa numbers, separated by space and representing the utf code of
// each characters of the wstring i.e. the wstring "totö要らない" will result in a string "74 6f 74 f6 8981 3089 306a
// 3044"
std::string s_w2h(std::wstring ws)
{
  std::stringstream cnv;
  for (auto e : ws)
  {
    // if (cnv.tellp() != 0)
    if (!cnv.str().empty())
      cnv << ' ';
    cnv << std::hex << (unsigned int)e;
  }

  return cnv.str();
}

// Convert a string containing a suite of hexa numbers, separated by space and representing utf code to a wstring
// i.e. the string "74 6f 74 f6 8981 3089 306a 3044"  will give the wstring "totö要らない"
std::wstring s_h2w(std::string hs)
{
  std::string hex_chars(hs);

  std::istringstream hex_chars_stream(hex_chars);
  std::wstring ws = L"";
  unsigned int c;
  while (hex_chars_stream >> std::hex >> c)
  {
    ws += (wchar_t)c;
  }

  return ws;
}

int main(int argc, char **argv)
{
#ifdef _WIN32
  _setmode(_fileno(stderr), _O_WTEXT);
#else
  std::setlocale(LC_ALL, "");
  std::locale::global(std::locale(""));
#endif
  std::filesystem::path p1 = u8"Hello, welcome to the wonderful world of utf (ütêf) 要らない.txt";
  std::string s1 = s_w2h(p1.wstring());
  std::filesystem::path p2 = s_h2w(s1);
  std::cout << p1 << std::endl;
}
