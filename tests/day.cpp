
#include <chrono>
#include <ctime>
#include <iostream>

/* Date de référence pour UNIX 1/1/1970 équivalent au résultat de date_to_julian_day(1970, 1, 1) */
#define UNIX_JULIAN_DAY 2440601

// (Julian day-UNIX_JULIAN_DAY) * 86400 + hour * 3600 + min * 60 + seconds == time_t
/* Date vers jour Julien */
int date_to_julian_day(int year, int month, int day)
{
  int a, y, m;

  a = (14 - month) / 12;
  y = year + 4800 - a;
  m = month + 12 * a - 3;

  return (day + (153 * m + 2) / 5 + y * 365 + y / 4 - 32083);
}

/* Jour julien vers date */
void julian_day_to_date(int jd, int &year, int &month, int &day)
{
  int c, d, e, m;

  c = jd + 32082;
  d = (4 * c + 3) / 1461;
  e = c - (1461 * d) / 4;
  m = (5 * e + 2) / 153;
  day = e - (153 * m + 2) / 5 + 1;
  month = m + 3 - 12 * (m / 10);
  year = d - 4800 + m / 10;
}

int date_to_time_stamp(int year, int mon, int day, int hour, int min, int sec)
{
  int my_julian_day = date_to_julian_day(year, mon, day);
  return (my_julian_day - UNIX_JULIAN_DAY) * 86400 + hour * 3600 + min * 60 + sec;
}

time_t date_time_to_time_t(int year, int mon, int day, int hour = 0, int min = 0, int sec = 0)
{
  return (time_t)date_to_time_stamp(year, mon, day, hour, min, sec);
}

std::string date_time_to_string(int year, int mon, int mday, int hour, int min, int sec)
{
  std::stringstream ss;
  ss << year << '/';
  ss << std::setfill('0') << std::setw(2) << mon << '/';
  ss << std::setfill('0') << std::setw(2) << mday;
  if (hour != 0 && min != 0 && sec != 0)
  {
    ss << ' ' << std::setfill('0') << std::setw(2) << hour << ':';
    ss << std::setfill('0') << std::setw(2) << min << ':';
    ss << std::setfill('0') << std::setw(2) << sec;
  }
  return ss.str();
}

std::string tm_to_string(tm tm)
{
  return date_time_to_string(1900 + tm.tm_year, 1 + tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
}

int days_count(int year)
{
  return 1 + date_to_julian_day(year, 12, 31) - date_to_julian_day(year, 1, 1);
}

bool isLeap(int year)
{
  return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0); /*
   if (year % 4 == 0)
   {
     if (year % 100 == 0 && year % 400 != 0)
       return false;
     else
       return true;
   }

   return false;*/
}

void yearWeekDay(tm TM, int &year, int &nweek, int &wday,
                 int &yday) // Reference: https://en.wikipedia.org/wiki/ISO_8601
{
  year = TM.tm_year + 1900;
  yday = TM.tm_yday;
  wday = TM.tm_wday;

  int monday = yday - (wday + 6) % 7;    // monday this week: may be negative down to 1-6 = -5;
  int mondayYear = 1 + (monday + 6) % 7; // First monday of the year
  int monday01 =
      (mondayYear > 4) ? mondayYear - 7 : mondayYear; // monday of week 1: should lie between -2 and 4 inclusive
  nweek = 1 + (monday - monday01) / 7;                // Nominal week ... but see below

  // In ISO-8601 there is no week 0 ... it will be week 52 or 53 of the previous year
  if (nweek == 0)
  {
    year--;
    nweek = 52;
    if (mondayYear == 3 || mondayYear == 4 || (isLeap(year) && mondayYear == 2))
      nweek = 53;
  }

  // Similar issues at the end of the calendar year
  if (nweek == 53)
  {
    int daysInYear = isLeap(year) ? 366 : 365;
    if (daysInYear - monday < 3)
    {
      year++;
      nweek = 1;
    }
  }

  yday++;
}

int main(int argc, char **argv)
{
  int year, nweek, wday, yday;

  // Par défaut on prend le timestamp actuel.
  time_t now = std::time(nullptr);
  tm tm;

  // Si un ou 2 arguments sont passé alors ils vont servir à changer la valeur de now, pour la date mais pas l'heure.
  if (argc > 1)
  {
    gmtime_s(&tm, &now);

    // 2 arguments: annéee et numéro du jour dans l'année.
    if (argc > 2)
    {
      year = atoi(argv[1]);
      yday = atoi(argv[2]);
    }
    else
    {
      // Un seul argument: numéro du jour dans l'année courante.
      year = 1900 + tm.tm_year;
      yday = atoi(argv[1]);
    }

    if (yday < 1 || yday > days_count(year))
    {
      std::cerr << "Day number " << yday << " for year " << year << " is not correct." << std::endl;
      return EXIT_FAILURE;
    }

    now = date_time_to_time_t(year, 1, 1) + (yday - 1) * 86400 + tm.tm_hour * 3600 + tm.tm_min * 60 + tm.tm_sec;
  }

  localtime_s(&tm, &now);

  yearWeekDay(tm, year, nweek, wday, yday);
  char str[26];
  asctime_s(str, sizeof str, &tm);
  std::cout << str;

  // Les jours d'une année se compte de 1 à 365 ou 366 pour les années bissextiles
  // Les jours d'une semaine se compte de 0 pour dimanche à 6 pour samedi
  // A titre d'exemple, le lundi 30 et le mardi 31 décembre 2024 font bien parti de la 1ére semaine de 2025 ...
  std::cout << "Day and week number in year: " << yday << ", " << nweek << std::endl;
  std::cout << "Day number in week: " << wday << std::endl;

  return EXIT_SUCCESS;
}
