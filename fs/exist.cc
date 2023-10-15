#include <filesystem>
#include <iostream>

int main()
{
   std::filesystem::path p("D:\\readme.txt");
   try
   {
      auto r = std::filesystem::exists(p);
      std::cout << "File " << p << " exists: " << r;
   }
   catch (const std::filesystem::filesystem_error& e)
   {
      std::cout << e.what();
   }
}
