#!/bin/bash

{
  echo -e '#ifndef WV_WM_MAP_H\n#define WV_WM_MAP_H\nstd::map<DWORD, std::string> wm_map = {'
  sed -n 's/^#define \(WM_[^[:space:]]*\).*/#ifdef \1%  { \1, "\1" },%#endif/p' /mingw64/include/winuser.h | sort -u | tr '%' '\n'
  echo -e '};\n#endif /* WV_WM_MAP_H */'
} >wv-wm_map.h

