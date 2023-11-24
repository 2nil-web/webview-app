
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// Convert a wstring to a string containing a suite of hexa numbers, separated by a space and representing the utf code
// of each characters of the wstring i.e. the wstring "totö要らない" will result in a string "74 6f 74 f6 8981 3089 306a
// 3044"
std::string s_w2h(std::wstring ws)
{
  std::stringstream cnv;
  bool not_first = false;
  for (auto e : ws)
  {
    if (not_first)
      cnv << ' ';
    else
      not_first = true;
    cnv << std::hex << (unsigned int)e;
  }

  return cnv.str();
}

// Convert a string containing a suite of hexa numbers, separated by a space and representing utf code to a wstring
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
  // Use of std::filesystem::path just to output the string but internally handle strings and wstrings
  std::filesystem::path ws;

  if (argc == 2)
    ws = argv[1];
  //  else ws=L"totö";
  else
    ws = L"totö要らない";

  std::cout << '[' << ws << ']' << std::endl;

  std::string hs = s_w2h(ws.wstring());
  std::cout << '[' << hs << ']' << std::endl;

  std::filesystem::path ws2 = s_h2w(hs);
  std::cout << '[' << ws2 << ']' << std::endl;

  return 0;
}
