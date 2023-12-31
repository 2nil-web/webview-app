
#include <chrono>
#include <format>
#include <iostream>

bool isLeap(int year)
{
  return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
}

int month_duration(int year, int month)
{
  switch (month)
  {
  case 2:
    if (isLeap(year))
      return 29;
    return 28;
  case 4:
  case 6:
  case 9:
  case 11:
    return 30;
  default:
    return 31;
  }
}


int main()
{
  std::locale("fr_FR.utf8");
  std::wcout << "User-preferred locale setting is " << std::locale("").name().c_str() << std::endl;
  std::chrono::year_month_day dmy;
  std::cin >> parse("%d %m %Y", dmy);
  std::cout << "dmy " << dmy << ", " << std::format("{:%A}", std::chrono::weekday{dmy}) << std::endl;
}
