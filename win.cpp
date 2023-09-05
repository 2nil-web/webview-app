
#include <windows.h>
#include <iostream>

void outwm (UINT msg) {
   std::cout << "msg " << msg << ", ";
    std::cout << "WM_CREATE " << WM_CREATE << ", ";
    std::cout << "WM_ACTIVATE " << WM_ACTIVATE << ", ";
    std::cout << "WM_SHOWWINDOW " << WM_SHOWWINDOW << ", ";
    std::cout << "WM_SETFOCUS " << WM_SETFOCUS << ", ";
    std::cout << "WM_SIZE " << WM_SIZE;
    std::cout << "WM_ENABLE " << WM_ENABLE;
    std::cout << std::endl;
}  

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  switch (msg) {
  case WM_NCCREATE :
    SetWindowPos(hwnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_HIDEWINDOW);
    ShowWindow(hwnd, SW_HIDE);
    return TRUE;
  case WM_DESTROY:
    PostQuitMessage(0);
    return 0;
  case WM_CREATE:
//  case WM_ACTIVATE:
  case WM_SHOWWINDOW:
  case WM_SETFOCUS:
  case WM_SIZE:
  case WM_ENABLE:
    outwm(msg);
    SetWindowPos(hwnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_HIDEWINDOW);
  //  ShowWindow(hwnd, SW_HIDE);
    return 0;
  default :
    //return CallWindowProc((WNDPROC)wpOrigEditProc, hwnd, msg, wParam, lParam);
    return DefWindowProc(hwnd, msg, wParam, lParam);
/*
  case WM_PAINT: {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);
     All painting occurs here, between BeginPaint and EndPaint.
    FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+1));
    EndPaint(hwnd, &ps);
    }
    return 0;
*/
  }
  return DefWindowProc(hwnd, msg, wParam, lParam);
}

HWND CreateWin() {
  const char CLASS_NAME[]="NoWebView";
  HINSTANCE currInst=(HINSTANCE)GetModuleHandle(NULL);
  WNDCLASS wc = { };
  wc.lpfnWndProc   = WindowProc;
  wc.hInstance     = currInst;
  wc.lpszClassName = "NoWebView";
  RegisterClass(&wc);
  HWND hwnd=CreateWindowEx(WS_EX_PALETTEWINDOW|WS_EX_NOACTIVATE, CLASS_NAME, "", WS_MINIMIZE, 10000, 0, 0, 0, NULL, NULL, currInst, NULL);
  if (hwnd == NULL) return 0;
  //ShowWindow(hwnd, SW_HIDE);

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
wpOrigEditProc=(WNDPROC)SetWindowLongPtr(wnd, GWLP_WNDPROC, (LONG_PTR)HideWindowProc);

LRESULT CALLBACK HideWindowProc(HWND hw, UINT msg, WPARAM wParam, LPARAM lParam) {
  switch (msg) {
  case WM_NCCREATE :
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
  default :
    //return CallWindowProc((WNDPROC)wpOrigEditProc, hw, msg, wParam, lParam);
    return DefWindowProc(hw, msg, wParam, lParam);
  }

//  return CallWindowProc((WNDPROC)wpOrigEditProc, hw, msg, wParam, lParam);
    return DefWindowProc(hw, msg, wParam, lParam);
}
HWND wnd;
#endif

