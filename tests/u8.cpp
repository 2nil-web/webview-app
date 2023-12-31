
#include <cstdio>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#else
#include <clocale>
#include <locale>
#endif
#include <filesystem>
#include <fstream>

int main()
{
#ifdef _WIN32
  _setmode(_fileno(stderr), _O_WTEXT);
#else
  std::setlocale(LC_ALL, "");
  std::locale::global(std::locale(""));
#endif

  std::filesystem::path p(u8"要らない.txt");
  std::ofstream(p) << "File contents: 要らない";
  // Prior to LWG2676 uses operator string_type()
  // on MSVC, where string_type is wstring, only
  // works due to non-standard extension.
  // Post-LWG2676 uses new fstream constructors

  // Native string representation can be used with OS-specific APIs
  std::FILE *f;
#ifdef _WIN32
  if (_wfopen_s(&f, p.c_str(), L"r") == 0)
#else
  if (std::FILE *f = std::fopen(p.c_str(), "r"))
#endif
  {
    for (int ch; (ch = fgetc(f)) != EOF; std::putchar(ch))
    {
    }
    std::fclose(f);
  }

  //    std::filesystem::remove(p);
}
