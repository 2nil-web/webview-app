
#include <iostream>
#include <chrono>
#include <ctime>

bool isLeap(int year) {
  if (year % 4 == 0) {
    if (year % 100 == 0 && year % 400 != 0) return false;
    else                                    return true;
  }

  return false;
}

void yearWeekDay(tm TM, int& year, int& nweek, int& wday, int& yday) // Reference: https://en.wikipedia.org/wiki/ISO_8601
{
  year = TM.tm_year + 1900;
  if (yday == -1) {
    yday = TM.tm_yday;
    wday = TM.tm_wday;
  } else {
    yday--;
    wday=yday%7;
  }

  int monday = yday - (wday + 6) % 7;                             // monday this week: may be negative down to 1-6 = -5;
  int mondayYear = 1 + (monday + 6) % 7;                          // First monday of the year
  int monday01 = (mondayYear > 4) ? mondayYear - 7 : mondayYear;  // monday of week 1: should lie between -2 and 4 inclusive
  nweek = 1 + (monday - monday01) / 7;                            // Nominal week ... but see below

  // In ISO-8601 there is no week 0 ... it will be week 52 or 53 of the previous year
  if (nweek == 0) {
   year--;
   nweek = 52;
   if (mondayYear == 3 || mondayYear == 4 || (isLeap(year) && mondayYear == 2)) nweek = 53;
  }

  // Similar issues at the end of the calendar year
  if (nweek == 53) {
   int daysInYear = isLeap(year) ? 366 : 365;
   if (daysInYear - monday < 3) {
     year++;
     nweek = 1;
   }
  }

  yday++;
}


int main(int argc, char **argv)
{
  int yday=-1;
  char dayname[7][4]= { "sun", "mon", "tue", "wed", "thu", "fri", "sat" };
  int year, nweek, wday;

  time_t now = time(0);
  tm *tm = localtime(&now);

  if (argc > 1) {
    yday=atoi(argv[1]);
    std::cout << "For year ";
  } else {
    std::cout << "For date " << tm->tm_mday << '/' << tm->tm_mon+1 << '/';
  }

  yearWeekDay(*tm, year, nweek, wday, yday);
  std::cout << year << ", day number in year: " << yday << ", in week: " << wday << ", name " << dayname[wday] << ", week number " << nweek << std::endl;

}

