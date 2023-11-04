
#ifndef WINAPI_PP_H
#define WINAPI_PP_H

#include <string>
#include <windows.h>

int MessageBox(HWND hWnd, LPCWSTR text, LPCWSTR caption, UINT uType = MB_OK);
int MessageBox(LPCWSTR text, LPCWSTR caption, UINT uType = MB_OK);
int MessageBox(HWND hWnd, const std::string text, const std::string caption, UINT uType = MB_OK);
int MessageBox(const std::string text, const std::string caption, UINT uType = MB_OK);
int MessageBox(HWND hWnd, const std::wstring text, const std::wstring caption, UINT uType = MB_OK);
int MessageBox(const std::wstring text, const std::wstring caption, UINT uType = MB_OK);

std::string ExpandEnvironmentStrings(const std::string src);
char *ExpandEnvironmentStrings(const char *src);

std::string GetCurrentDirectory();

std::string GetDlgItemText(HWND hDlg, int nIDDlgItem);
std::string GetWindowText(HWND hDlg);
bool SetWindowText(HWND hWnd, std::string text);
bool SetDlgItemText(HWND hDlg, int nIDDlgItem, std::string text);
#endif /* WINAPI_PP_H */
