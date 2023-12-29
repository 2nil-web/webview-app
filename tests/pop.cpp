
#include <cstdio>
#include <iostream>
#include <sstream>
#include <string>

#include "../wv-util.cpp"
#include "../wv-util.h"

// int main(int argc, char **argv)
int main()
{
  int argc;
  wchar_t **argv;
  SetConsoleOutputCP(CP_UTF8);
  argv = CommandLineToArgvW(GetCommandLineW(), &argc);
  for (int i = 1; i < argc; i++)
  {
    std::wcout << argv[i] << ':' << std::endl;
    std::cout << wpipe2s(argv[i]) << std::endl;
  }
}
