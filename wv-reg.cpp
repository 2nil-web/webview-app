
#include <string>
#include <cstdio>
#include <iostream>
#include <windows.h>

#include "wv-winapi.h"
#include "wv-reg.h"

#define MAX_STR 2048
#define THE_HKEY HKEY_CURRENT_USER

HKEY RegOpen(const char *subKey)
{
  HKEY hkRes;
  LONG ret;
  DWORD dwDisp;

  ret = RegCreateKeyEx(THE_HKEY, subKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &hkRes, &dwDisp);
  if (ret != ERROR_SUCCESS)
    WinError("RegCreateKeyEx");

  std::cout << "RegOpen hkey " << THE_HKEY << ", subKey " << subKey << std::endl;
  return hkRes;
}

int GetRegInt(const char *subKey, const char *var, int defval)
{
  LONG ret;
  HKEY hk = RegOpen(subKey);
  DWORD type, val, lval = sizeof(DWORD);

  ret = RegQueryValueEx(hk, var, NULL, &type, (LPBYTE)&val, &lval);
  RegCloseKey(hk);

  if (ret != ERROR_SUCCESS || type != REG_DWORD)
    return defval;
  return val;
}

// size contient la taille attendue ou z?ro si indiff?rent
int GetRegBin(const char *subKey, const char *var, DWORD size, BYTE *val)
{
  HKEY hk = RegOpen(subKey);
  DWORD type, gsize;

  if (RegQueryValueEx(hk, var, NULL, &type, NULL, &gsize) != ERROR_SUCCESS || type != REG_BINARY || size != gsize)
  {
    RegCloseKey(hk);
    return 0;
  }

  RegQueryValueEx(hk, var, NULL, &type, val, &size);
  RegCloseKey(hk);
  return 1;
}

int *GetRegIntArray(const char *subKey, const char *var, int *size)
{
  HKEY hk = RegOpen(subKey);
  int lsize = 0;
  size_t i;
  char str[MAX_STR], *tok, *itok;
  DWORD lstr = MAX_STR;
  DWORD type;

  if (RegQueryValueEx(hk, var, NULL, &type, (LPBYTE)str, &lstr) != ERROR_SUCCESS || type != REG_SZ)
  {
    *size = -1;
    return NULL;
  }

  RegCloseKey(hk);

  for (i = 0; i < strlen(str); i++)
    if (str[i] == ' ')
      lsize++;
  //  lsize++;

  //  val=(int *)malloc(lsize*sizeof(int));
  auto val = new int[lsize];

  i = 0;
  tok = strtok_s(str, " ", &itok);
  while (tok)
  {
    val[i] = atoi(tok);
    i++;
    tok = strtok_s(NULL, " ", &itok);
  }

  *size = (int)i;

  return val;
}

std::string GetRegString(const std::string subKey, const std::string var, const std::string defval)
{
  std::string ret("");
  HKEY hk = RegOpen(subKey.c_str());
  DWORD type, lval;

  if (RegQueryValueEx(hk, var.c_str(), NULL, &type, NULL, &lval) == ERROR_MORE_DATA && type == REG_SZ) {
    auto val = new BYTE[lval];

    RegQueryValueEx(hk, var.c_str(), NULL, &type, val, &lval);
    ret = std::string((char *)val);
  } else ret = defval;

  std::cout << "GetRegString key " << subKey << ", var " << var << ", val " << ret << std::endl;

  RegCloseKey(hk);
  return ret;
}

std::string GetRegExpString(const std::string subKey, const std::string var, const std::string defval)
{
  std::string ret("");
  HKEY hk = RegOpen(subKey.c_str());
  DWORD type = REG_EXPAND_SZ, lval;

  if (RegQueryValueEx(hk, var.c_str(), NULL, &type, NULL, &lval) == ERROR_SUCCESS && type == REG_EXPAND_SZ)
  {
    auto val = new BYTE[lval];

    RegQueryValueEx(hk, var.c_str(), NULL, &type, val, &lval);
    ret = std::string((char *)val);
  }
  else
    ret = defval;

  RegCloseKey(hk);
  return ret;
}

int GetRegInt(const std::string subKey, const std::string var, const int defval)
{
  return GetRegInt(subKey.c_str(), var.c_str(), defval);
}

bool GetRegBin(const std::string subKey, const std::string var, DWORD size, BYTE *val)
{
  return GetRegBin(subKey.c_str(), var.c_str(), size, val);
}

int *GetRegIntArray(const std::string subKey, const std::string var, int *size)
{
  return GetRegIntArray(subKey.c_str(), var.c_str(), size);
}

void PutRegString(const char *subKey, const char *var, const char *val)
{
  HKEY hk = RegOpen(subKey);
  std::cout << "PutRegString key " << subKey << ", var " << var << ", val " << val << std::endl;
  RegSetValueEx(hk, var, 0, REG_SZ, (LPBYTE)val, (DWORD)strlen(val) + 1);
  RegCloseKey(hk);
}

void PutRegString(const std::string subKey, const std::string var, const std::string val)
{
  PutRegString(subKey.c_str(), var.c_str(), val.c_str());
}


void PutRegInt(const char *subKey, const char *var, DWORD val)
{
  HKEY hk = RegOpen(subKey);
  RegSetValueEx(hk, var, 0, REG_DWORD, (LPBYTE)&val, sizeof(DWORD));
  RegCloseKey(hk);
}

/* Sauve un tableau d'entier comme une chaine dans la registry */
void PutRegIntArray(const char *subKey, const char *var, int size, int *val)
{
  std::string str = "";
  int i;
  HKEY hk;

  for (i = 0; i < size; i++)
  {
    if (str.size() + 10 > MAX_STR)
      break;
    str = str + std::to_string(val[i]);
    //    sprintf(str, "%s %d", str, val[i]);
  }

  hk = RegOpen(subKey);
  RegSetValueEx(hk, var, 0, REG_SZ, (LPBYTE)str.c_str(), (DWORD)str.size() + 1);
  RegCloseKey(hk);
}

void PutRegBin(const char *subKey, const char *var, DWORD size, CONST BYTE *val)
{
  HKEY hk = RegOpen(subKey);
  RegSetValueEx(hk, var, 0, REG_BINARY, val, size);
  RegCloseKey(hk);
}

void PutRegInt(const std::string subKey, const std::string var, DWORD val)
{
  PutRegInt(subKey.c_str(), var.c_str(), val);
}

void PutRegIntArray(const std::string subKey, const std::string var, int size, int *val)
{
  PutRegIntArray(subKey.c_str(), var.c_str(), size, val);
}

void PutRegBin(const std::string subKey, const std::string var, DWORD size, CONST BYTE *val)
{
  PutRegBin(subKey.c_str(), var.c_str(), size, val);
}
