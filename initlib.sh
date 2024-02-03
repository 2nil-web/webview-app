#!/bin/bash

echo "Prerequisite are git and a c++ compiler that can be g++ or Visual Studio."
echo "To generate the application icon you may need magick convert or provide an icon file named webview-app.ico"

if ! which git >/dev/null 2>&1
then
  echo "You need git to download the webview library."
  exit
fi

WVDIR=webview

echo -n "Considering the use of webview with "

[ "$WVDIR" = "webview-vs" ] && echo "Visual Studio." || echo "the gcc/g++ compiler suite."

if [ -d ${WVDIR} ]
then
  echo "No need to clone the $WVDIR directory as it already exists."
else
  git clone https://github.com/webview/webview.git ${WVDIR}
fi

${WVDIR}/script/build.sh

#cp ${WVDIR}/webview.h my_webview.h
#cp ${WVDIR}/webview_mingw_support.h .
cp ${WVDIR}/build/external/libs/Microsoft.Web.WebView2*/build/native/include/* .
echo -n "You can now compile this example by running "
[ "$WVDIR" = "webview-vs" ] && echo "Visual Studio." || echo "the make command."


