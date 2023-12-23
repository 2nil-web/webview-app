
#include <iostream>
#ifdef _WIN32
#include <windows.h>
#endif

int main()
{
#ifdef _WIN32
  SetConsoleOutputCP(CP_UTF8);
#endif
  std::wstring s = L"";
  for (wchar_t i = 32; i < 255; i++)
  {
    if (isascii(i))
    {
      std::cout << "ISASCII " << (int)i;
      std::wcout << ", " << i << std::endl;
    }
    s += i;
  }

  std::wcout << s << std::endl;
}
