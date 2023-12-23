// Main.cpp
#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif
#include <windows.h>
#define IDC_LABEL1 2000

LRESULT CALLBACK fnWndProc(HWND hWnd, unsigned int msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg)
  {
  case WM_CREATE: {
    HINSTANCE hIns = ((LPCREATESTRUCT)lParam)->hInstance;
    HWND hCtl = CreateWindowEx(0, L"static", L"Label1", WS_CHILD | WS_VISIBLE, 40, 40, 80, 30, hWnd, (HMENU)IDC_LABEL1,
                               hIns, 0);
    return 0;
  }
  case WM_CTLCOLORSTATIC:                              // Do not allow DefWindowProc() to process this message!
  {                                                    // When a WM_CTLCOLORSTATIC message comes through, return
    SetBkMode((HDC)wParam, TRANSPARENT);               // from the Window Procedure call without a DefWindowProc()
    return GetWindowLongPtr(hWnd, 0 * sizeof(void *)); // call. Instead return the HBRUSH stored as an instance
  }                                                    // variable as part of the WNDCLASSEX object.
  case WM_DESTROY: {
    PostQuitMessage(0);
    return 0;
  }
  }

  return (DefWindowProc(hWnd, msg, wParam, lParam));
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevIns, LPSTR lpszArgument, int iShow)
{
  wchar_t szClassName[] = L"Form1";
  WNDCLASSEX wc;
  MSG messages;
  HWND hWnd;

  memset(&wc, 0, sizeof(WNDCLASSEX));
  HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 222)); // Create a non-standard color HBRUSH for main window
  wc.lpszClassName = szClassName;
  wc.lpfnWndProc = fnWndProc;
  wc.cbSize = sizeof(WNDCLASSEX);
  wc.hbrBackground = hBrush;
  wc.hInstance = hInstance;
  wc.cbWndExtra = sizeof(void *);
  RegisterClassEx(&wc);
  hWnd = CreateWindowEx(0, szClassName, szClassName, WS_OVERLAPPEDWINDOW, 200, 175, 320, 200, HWND_DESKTOP, 0,
                        hInstance, 0);
  SetWindowLongPtr(hWnd, 0 * sizeof(void *), (LONG_PTR)hBrush); // Store HBRUSH as part of Window Class Structure
  ShowWindow(hWnd, iShow);
  while (GetMessage(&messages, NULL, 0, 0))
  {
    TranslateMessage(&messages);
    DispatchMessage(&messages);
  }
  DeleteObject(hBrush); // Delete dynamically allocated GDI resource

  return messages.wParam;
}
