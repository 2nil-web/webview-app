
#include <filesystem>
#include <fstream>
#include <iostream>

void demo_perms(std::filesystem::path f)
{
  std::filesystem::perms p = std::filesystem::status(f).permissions();
  using std::filesystem::perms;

  auto show = [=](char op, perms perm) { std::cout << (perms::none == (perm & p) ? '-' : op); };

  std::cout << f << ':' << std::oct << (unsigned int)p << ':';
  show('r', perms::owner_read);
  show('w', perms::owner_write);
  show('x', perms::owner_exec);
  show('r', perms::group_read);
  show('w', perms::group_write);
  show('x', perms::group_exec);
  show('r', perms::others_read);
  show('w', perms::others_write);
  show('x', perms::others_exec);
  std::cout << std::endl;
}

int main()
{
  // std::ofstream("test.txt"); // create file
  std::cout << "Created file with permissions: ";
  demo_perms("test.txt");
  std::filesystem::permissions("test.txt", std::filesystem::perms::owner_all | std::filesystem::perms::group_all,
                               std::filesystem::perm_options::add);
  std::cout << "After adding u+rwx and g+rwx:  ";
  demo_perms("test.txt");
  demo_perms("isascii.cpp");

  // std::filesystem::remove("test.txt");
}
