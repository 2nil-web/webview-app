
#include <array>
#include <charconv>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <windows.h>

std::string toch(auto... format_args)
{
  std::array<char, 10> str;
  std::string res = "";

#if __cpp_lib_to_chars >= 202306L
  // use C++26 operator bool() for error checking
  if (!(res = std::to_chars(str.data(), str.data() + str.size(), format_args...)))
    std::cout << std::make_error_code(res.ec).message() << std::endl;
#else
  if (auto [ptr, ec] = std::to_chars(str.data(), str.data() + str.size(), format_args...); ec == std::errc())
    res = std::string(str.data(), ptr);
  // std::cout << std::string_view(str.data(), ptr) << std::endl;
  else
    std::cout << std::make_error_code(ec).message() << std::endl;
#endif
  return res;
}

template <class T> std::string int_to_chars(T d, int base = 10)
{
  return toch(d, base);
}

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
      std::cout << "sizeof(c)  " << sizeof(c) << ", sizeof(ui) " << sizeof(ui) << std::endl;
      one = false;
    }
    if (ui > 32 && ui < 127)
    {
      res += (char)c;
    }
    else
    {
      cnv.str("");
      cnv << ' ' << int_to_chars(ui, 16);
      res += cnv.str();
    }
  }

  return res;
}

int main()
{
  SetConsoleOutputCP(CP_UTF8);
  std::cout << "sizeof(std::wstring) " << sizeof(std::wstring) << std::endl;
  std::wstring ws = L"string=Ā😀ドイツ語で検索していてこちらのサイトにたどり着きました。";
  std::cout << Ts2h(ws) << std::endl;

  std::cout << "sizeof(std::string)  " << sizeof(std::string) << std::endl;
  std::string s = "string=Ā😀ドイツ語で検索していてこちらのサイトにたどり着きました。";
  std::cout << Ts2h(s) << std::endl;

  return 0;
}

// MSVC
// string= 196 8364 240 376 732 8364 227 402 8240 227 8218 164 227 402 8222 232 170 382 227 129 167 230 164 339 231 180
// 162 227 129 8212 227 129 166 227 129 8222 227 129 166 227 129 8220 227 129 161 227 8218 8240 227 129 174 227 string=
// c4 20ac f0 178 2dc 20ac e3 192 2030 e3 201a a4 e3 192 201e e8 aa 17e e3 81 a7 e6 a4 153 e7 b4 a2 e3 81 2014 e3 81 a6
// e3 81 201e e3 81 a6 e3 81 201c e3 81 a1 e3 201a 2030 e3 81 ae e3 201a b5 e3 201a a4 e3 192 2c6 e3 81 ab e3 81 178 e3
// 81 a9 e3 201a 160 e7 9d 20ac e3 81 8d e3 81 be e3 81 2014 e3 81 178 e3 20ac 201a GCC string= 256 55357 56832 12489
// 12452 12484 35486 12391 26908 32034 12375 12390 12356 12390 12371 12385 12425 12398 12469 12452 12488 12395 12383
// 12393 12426 30528 12365 12414 12375 12383 12290 string= 100 d83d de00 30c9 30a4 30c4 8a9e 3067 691c 7d22 3057 3066
// 3044 3066 3053 3061 3089 306e 30b5 30a4 30c8 306b 305f 3069 308a 7740 304d 307e 3057 305f 3002
