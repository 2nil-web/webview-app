
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <list>
#include <string>
#include <vector>

bool isWideString(const std::string &s)
{
  for (auto &c : s)
  {
    if (c & 0x80)
      return true;
  }

  return false;
}

std::string skipWideChars(std::string ws)
{
  std::string s;
  for (auto &c : ws)
  {
    if (c & 0x80)
      s += c;
    else
      s += '*';
  }

  return s;
}

void testWideString(std::string s)
{
  bool isws = isWideString(s);
  std::cout << s << " is" << (isws ? " " : " not ") << "a wide string" << std::endl;
  if (isws)
    std::cout << skipWideChars(s) << std::endl;
}

int main(int argc, char *argv[])
{
  // Convertis les arguments en un vecteur de chaine de caractères.
  std::vector<std::string> args(argv, argv + argc);
  if (args.size() <= 1)
  {
    testWideString("###_要らない_###");
    testWideString("###_éèàôû_###");
  }
  else
    testWideString(args[1]);

  return 0;
}
