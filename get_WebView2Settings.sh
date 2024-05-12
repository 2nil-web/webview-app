#!/bin/bash

function settng () {
  echo
  echo " // Settings$1"
  sed "s/ettings/ettings$1/g" <<EOF
  ICoreWebView2Settings *settings = nullptr;
  res = mwv->get_Settings(&settings);
  if (res != S_OK) {
    std::cerr << "WEBVIEW_ERROR_UNSPECIFIED, get_Settings failed" << std::endl;
    return false;
  }
EOF
}

function get() {
last_n=0
#settng
sed -n 's/^#define ICoreWebView2Settings\(.*\)_get_/settings\1->get_/p' WebView2.h | sed 's/This,.*$/\&is_set);/;s/get_\(.*\)\((&.*$\)/get_\1\2 disp_set("\1", is_set);/' | while read ln
do
  n=${ln/settings}
  n=${n/->*}

  if false
  then
    echo $ln

    if [ ! "$last_n" = "$n" ]
    then
      settng $n
      last_n=$n
    fi
  else
    if [ ! "$last_n" = "$n" ]
    then
      echo
      echo $n
    fi
    echo $ln
  fi
done
}

function get2 () {
  cat <<EOF
bool is_set(HRESULT (*wv2get_set)(BOOL *), std::string sset) {
  BOOL is_set;
  auto res=wv2get_set(&is_set);
  if (res == S_OK) {
    std::cout << sset << " is" << (is_set?"":"not ") << "set" << std::endl;
  } else {
    std::cout << sset << " feature does not exist" << std::endl;
  }
}

void all_wv_set () {
EOF

  echo "  // Settings$1"
  sed "s/ettings/ettings$1/g" <<EOF
  ICoreWebView2Settings *settings = nullptr;
  res = mwv->get_Settings(&settings);
  if (res != S_OK) {
    std::cerr << "WEBVIEW_ERROR_UNSPECIFIED, get_Settings failed" << std::endl;
    return false;
  }
EOF

#sed -n "s/^#define ICoreWebView2Settings$1_get_\(.*\)(.*/  settings$1->get_\1(\&is_set); disp_set(\"\1\", is_set);/p" WebView2.h
sed -n "s/^#define ICoreWebView2Settings$1_get_\(.*\)(.*/  do_set(settings$1->get_\1, \"\1\");/p" WebView2.h
echo "}"
}

get2 9
