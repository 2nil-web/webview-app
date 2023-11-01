
#include <iostream>

std::string utf16_to_utf8( std::u16string&& utf16_string )
{
   std::wstring_convert<std::codecvt_utf8_utf16<int16_t>, int16_t> convert;
   auto p = reinterpret_cast<const int16_t *>( utf16_string.data() );
   return convert.to_bytes( p, p + utf16_string.size() );
}

int main(int argc, char **arv) {
  std::string    str1 =  "This is a String";
  std::wstring   str2 = L"This is a String";
  std::u16string str3 = u"This is a String";
  std::u32string str4 = U"This is a String";

  std::pmr::string    pstr1 =  "This is a String";
  std::pmr::wstring   pstr2 = L"This is a String";
  std::pmr::u16string pstr3 = u"This is a String";
  std::pmr::u32string pstr4 = U"This is a String";

//  std::vector<std::u16string> args(argv, argv + argc);
//  if (args.size() <= 1) {
//    std::cout << utf16_to_utf8(u"###_要らない_###");
//    std::cout << ", ";
//    std::cout << utf16_to_utf8(u"###_éèàôû_###");
//  } else std::cout << utf16_to_utf8(args[1]);

  std::cout <<  << std::endl;

  return 0;
}

