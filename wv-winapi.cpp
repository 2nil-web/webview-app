
#include <windows.h>

#include "wv-winapi.h"

#include <filesystem>
#include <iostream>

std::string StrError(const char *fmt, ...)
{
  CHAR *lpMsgBuf;
  char title[1024];
  DWORD len;
  va_list ap;

  va_start(ap, fmt);
  vsnprintf(title, 1024, fmt, ap);
  va_end(ap);

  len = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
                      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
  lpMsgBuf[len - 2] = '\0';
  std::string ret=title;
  ret=ret+" - "+lpMsgBuf;
  LocalFree(lpMsgBuf);

  return ret;
}

void WinError(const char *fmt, ...)
{
  CHAR *lpMsgBuf;
  char title[1024];
  DWORD len;
  va_list ap;

  va_start(ap, fmt);
  vsnprintf(title, 1024, fmt, ap);
  va_end(ap);

  len = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
                      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
  lpMsgBuf[len - 2] = '\0';
  MessageBox(NULL, lpMsgBuf, title, MB_OK | MB_ICONERROR);
  LocalFree(lpMsgBuf);
}

#ifndef UNICODE
PCHAR *CommandLineToArgvA(PCHAR CmdLine, int *_argc)
{
  PCHAR *argv;
  PCHAR _argv;
  ULONG len;
  ULONG argc;
  CHAR a;
  ULONG i, j;

  BOOLEAN in_QM;
  BOOLEAN in_TEXT;
  BOOLEAN in_SPACE;

  len = (ULONG)strlen(CmdLine);
  i = ((len + 2) / 2) * sizeof(PVOID) + sizeof(PVOID);
  argv = (PCHAR *)GlobalAlloc(GMEM_FIXED, i + (len + 2) * sizeof(CHAR));
  _argv = (PCHAR)(((PUCHAR)argv) + i);
  argc = 0;
  argv[argc] = _argv;
  in_QM = FALSE;
  in_TEXT = FALSE;
  in_SPACE = TRUE;
  i = 0;
  j = 0;

  while ((a = CmdLine[i]))
  {
    if (in_QM)
    {
      if (a == '\"')
      {
        in_QM = FALSE;
      }
      else
      {
        _argv[j] = a;
        j++;
      }
    }
    else
    {
      switch (a)
      {
      case '\"':
        in_QM = TRUE;
        in_TEXT = TRUE;
        if (in_SPACE)
        {
          argv[argc] = _argv + j;
          argc++;
        }
        in_SPACE = FALSE;
        break;
      case ' ':
      case '\t':
      case '\n':
      case '\r':
        if (in_TEXT)
        {
          _argv[j] = '\0';
          j++;
        }
        in_TEXT = FALSE;
        in_SPACE = TRUE;
        break;
      default:
        in_TEXT = TRUE;
        if (in_SPACE)
        {
          argv[argc] = _argv + j;
          argc++;
        }
        _argv[j] = a;
        j++;
        in_SPACE = FALSE;
        break;
      }
    }
    i++;
  }
  _argv[j] = '\0';
  argv[argc] = nullptr;

  (*_argc) = argc;
  return argv;
}
#endif

int MessageBox(LPCSTR text, LPCSTR caption, UINT type)
{
  return MessageBox(NULL, text, caption, type);
}

int MessageBox(HWND hwnd, const std::string text, const std::string caption, UINT type)
{
  return MessageBox(hwnd, text.c_str(), caption.c_str(), type);
}

int MessageBox(const std::string text, const std::string caption, UINT type)
{
  return MessageBox(NULL, text, caption, type);
}

int MessageBox(HWND hwnd, const std::wstring text, const std::wstring caption, UINT type)
{
  return MessageBoxW(hwnd, text.c_str(), caption.c_str(), type);
}

int MessageBox(const std::wstring text, const std::wstring caption, UINT type)
{
  return MessageBox(NULL, text, caption, type);
}

std::string ExpandEnvironmentStrings(const std::string s)
{
  DWORD l = ExpandEnvironmentStrings(s.c_str(), NULL, 0);
  std::string ret("");

  if (l > 0)
  {
    char *d = new char[l + 1];
    if (ExpandEnvironmentStrings(s.c_str(), d, l) > 0)
      ret = d;
  }

  return ret;
}

char *ExpandEnvironmentStrings(const char *src)
{
  return _strdup(ExpandEnvironmentStrings(std::string(src)).c_str());
}

std::string GetCurrentDirectory()
{
#ifndef PURE_WINAPI
  return std::filesystem::current_path().string();
#else
  std::string ret = "";
  DWORD l = GetCurrentDirectory(0, NULL);

  if (l > 0)
  {
    char buf[l + 1];
    GetCurrentDirectory(l, buf);
    ret = buf;
  }

  return ret;
#endif
}

std::string GetWindowText(HWND hwnd)
{
  std::string ret("");
  int l = GetWindowTextLength(hwnd);

  if (l > 0)
  {
    char *s = new char[l + 2];
    if (GetWindowText(hwnd, s, l + 1) > 0)
    {
      ret = std::string(s);
    }
  }

  return ret;
}

std::string GetDlgItemText(HWND hDlg, int nIDDlgItem)
{
  return GetWindowText(GetDlgItem(hDlg, nIDDlgItem));
}

std::string GetDlgItemTextRES(HWND hDlg, int nIDDlgItem)
{
  std::string ret("");
  int l = (int)SendDlgItemMessage(hDlg, nIDDlgItem, WM_GETTEXTLENGTH, 0, 0);

  if (l > 0)
  {
    char *s = new char[l + 2];
    GetDlgItemText(hDlg, nIDDlgItem, s, l + 1);
    printf("gdit s [%s]\n", s);
    ret = std::string(s);
  }

  return ret;
}

bool SetWindowText(HWND hWnd, std::string text)
{
  return SetWindowText(hWnd, text.c_str());
}

bool SetDlgItemText(HWND hDlg, int nIDDlgItem, std::string text)
{
  return SetDlgItemText(hDlg, nIDDlgItem, text.c_str());
}

void outwm(UINT msg)
{
  std::cout << "msg " << msg << ", ";
  std::cout << "WM_CREATE " << WM_CREATE << ", ";
  std::cout << "WM_ACTIVATE " << WM_ACTIVATE << ", ";
  std::cout << "WM_SHOWWINDOW " << WM_SHOWWINDOW << ", ";
  std::cout << "WM_SETFOCUS " << WM_SETFOCUS << ", ";
  std::cout << "WM_SIZE " << WM_SIZE;
  std::cout << "WM_ENABLE " << WM_ENABLE;
  std::cout << std::endl;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg)
  { /*
case WM_NCCREATE :
SetWindowPos(hwnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_HIDEWINDOW);
ShowWindow(hwnd, SW_HIDE);
return TRUE;*/
  case WM_DESTROY:
    PostQuitMessage(0);
    return 0;
  // case WM_CREATE:
  // case WM_SHOWWINDOW:
  // case WM_SETFOCUS:
  // case WM_ENABLE:
  case WM_SIZE:
    // outwm(msg);
    SetWindowPos(hwnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_HIDEWINDOW);
    ShowWindow(hwnd, SW_HIDE);
    return 0;
  default:
    // return CallWindowProc((WNDPROC)wpOrigEditProc, hwnd, msg, wParam,
    // lParam);
    return DefWindowProc(hwnd, msg, wParam, lParam);
  }
  return DefWindowProc(hwnd, msg, wParam, lParam);
}

HWND CreateWin()
{
  const char CLASS_NAME[] = "NoWebView";
  HINSTANCE currInst = (HINSTANCE)GetModuleHandle(NULL);
  WNDCLASS wc = {};
  wc.lpfnWndProc = WindowProc;
  wc.hInstance = currInst;
  wc.lpszClassName = "NoWebView";
  RegisterClass(&wc);
  HWND hwnd = CreateWindowEx(WS_EX_PALETTEWINDOW | WS_EX_NOACTIVATE, CLASS_NAME, "", WS_MINIMIZE, 10000, 0, 0, 0, NULL,
                             NULL, currInst, NULL);
  if (hwnd == NULL)
    return 0;
  // ShowWindow(hwnd, SW_HIDE);

  /*
  MSG msg = { };
  while (GetMessage(&msg, NULL, 0, 0) > 0) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
  }

  return 0;*/
  return hwnd;
}

#ifdef RES
WNDPROC wpOrigEditProc;
wpOrigEditProc = (WNDPROC)SetWindowLongPtr(wnd, GWLP_WNDPROC, (LONG_PTR)HideWindowProc);

LRESULT CALLBACK HideWindowProc(HWND hw, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg)
  {
  case WM_NCCREATE:
    SetWindowPos(hw, HWND_BOTTOM, 0, 0, 0, 0, SWP_HIDEWINDOW);
    ShowWindow(hw, SW_HIDE);
    return TRUE;
  case WM_CREATE:
    return -1;
  case WM_ACTIVATE:
  case WM_SHOWWINDOW:
  case WM_SETFOCUS:
  case WM_SIZE:
  case WM_ENABLE:
    SetWindowPos(hw, HWND_BOTTOM, 0, 0, 0, 0, SWP_HIDEWINDOW);
    ShowWindow(hw, SW_HIDE);
    std::cout << "msg " << msg << ", ";
    std::cout << "WM_CREATE " << WM_CREATE << ", ";
    std::cout << "WM_ACTIVATE " << WM_ACTIVATE << ", ";
    std::cout << "WM_SHOWWINDOW " << WM_SHOWWINDOW << ", ";
    std::cout << "WM_SETFOCUS " << WM_SETFOCUS << ", ";
    std::cout << "WM_SIZE " << WM_SIZE;
    std::cout << "WM_ENABLE " << WM_ENABLE;
    std::cout << std::endl;
    return 0;
  default:
    return CallWindowProc((WNDPROC)wpOrigEditProc, hw, msg, wParam, lParam);
    //    return DefWindowProc(hw, msg, wParam, lParam);
  }

  return CallWindowProc((WNDPROC)wpOrigEditProc, hw, msg, wParam, lParam);
  //    return DefWindowProc(hw, msg, wParam, lParam);
}
HWND wnd;
#endif
