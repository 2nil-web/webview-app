@echo off

echo Prerequisite are git and the Visual Studio compiler suite for C++.
echo To generate the application icon you may need magick convert or provide an icon file named webview-app.ico.

where git >nul || ( echo You need git to download the webview library.; exit /b )

where cmake >nul || ( echo You need cmake to build the webview library.; exit /b )

echo Considering the use of webview with Visual Studio.

if exist webview-vs\ (
  echo No need to clone the webview-vs directory as it already exists.
) else (
  echo Cloning webview-vs ...
  git clone https://github.com/webview/webview.git webview-vs
)

cmd /C webview-vs\script\build.bat

echo You can now compile this example by running Visual Studio.

