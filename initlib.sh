#!/bin/bash

echo "Prerequisite are git and a c++ compiler that can be g++ or Visual Studio."
echo "To generate the application icon you may need magick convert or provide an icon file named webview-test.ico"

if ! which git >/dev/null 2>&1
then
  echo "You need git to download the webview library."
  exit
fi

WVDIR=webview-gcc

if [[ "${OSTYPE}" == "msys" || "${OSTYPE}" == "cygwin" ]]
then
  if ! which make g++ >/dev/null 2>&1
  then
    echo "Considering the use of webview with Visual Studio."
    WVDIR=webview-vs
    echo "Considering the use of webview with the gcc/g++ compiler."
    WVDIR=webview-gcc
  fi
fi

echo -n "Considering the use of webview with "

[ "$WVDIR" = "webview-vs" ] && echo "Visual Studio." || echo "the gcc/g++ compiler suite."

if [ -d ${WVDIR} ]
then
  echo "No need to clone the $WVDIR directory as it already exists."
else
  git clone https://github.com/webview/webview.git ${WVDIR}
fi

${WVDIR}/script/build.sh

echo -n "You can now compile this example by running "
[ "$WVDIR" = "webview-vs" ] && echo "Visual Studio." || echo "the make command."


