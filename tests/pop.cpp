
#include <cstdio>
#include <iostream>
#include <string>
#include <sstream>

#include "../wv-util.h"
#include "../wv-util.cpp"

//int main(int argc, char **argv)
int main()
{
  int argc; wchar_t **argv;
  SetConsoleOutputCP(CP_UTF8);
  argv=CommandLineToArgvW(GetCommandLineW(), &argc);
  for (int i=1; i < argc; i++) {
    std::wcout << argv[i] << ':' << std::endl;
    std::cout << wpipe2s(argv[i]) << std::endl;
  }
}

