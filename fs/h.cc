
#include <iostream>
#include <string>
#include <sstream> 
#include <vector>
#include <filesystem>

int main() {
  //std::string hex_chars("E8 48 D8 FF FF 8B 0D");
  //std::string hex_chars("4C 41 4C 41 4E 4E FB");
  std::string hex_chars("74 6f 74 6f 8981 3089 306a 3044");

  std::istringstream hex_chars_stream(hex_chars);
  std::vector<unsigned int> bytes;
  std::wstring ws=L"";
  unsigned int c;
  while (hex_chars_stream >> std::hex >> c) {
    bytes.push_back(c);
    ws+= (wchar_t)c;
  }

  std::filesystem::path p=ws;
  std::cout << p << std::endl;
  return 0;
}
