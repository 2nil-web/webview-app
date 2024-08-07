
# Bug d'affichage avec uname sous clang64 ...
ifeq (${MSYSTEM},CLANG64)
	UNAME=/usr/bin/uname
else
	UNAME=uname
endif

OS=$(shell ${UNAME} -s)

#WITH_CURL=1
# intégrer js check + minify

ifneq (${WITH_CURL},)
	CPPFLAGS += -DWITH_CURL
ifeq ($(STATIC_CURL),1)
	CPPFLAGS += -DCURL_STATICLIB
endif
ifeq (${MSYSTEM},UCRTW64)
	STATIC_CURL=0
else
	STATIC_CURL=1
endif

STATIC_CURL=0
endif


ifneq (${OS},Linux)
#export MSYSTEM=CLANG64
# Compile en clang64 ou ucrt64 mais pas mingw64
#export GCC_PATH=/clang64/bin
#export GCC_PATH=/ucrt64/bin
CPPFLAGS += -DWIN32_LEAN_AND_MEAN
export GCC_PATH=/mingw64/bin
export PATH:=${GCC_PATH}:${PATH}

# Bug d'affichage avec uname sous clang64 ...
ifeq (${MSYSTEM},CLANG64)
	CC=${GCC_PATH}/clang
	CXX=${GCC_PATH}/clang++
else
	CC=${GCC_PATH}/gcc
	CXX=${GCC_PATH}/g++
endif

	ECHO=echo -e
	PGF=$(subst \,/,$(subst C:\,/c/,$(PROGRAMFILES)))
	PGF86=${PGF} (x86)
	LOCAD=$(subst \,/,$(subst C:\,/c/,$(LOCALAPPDATA)))
	PGU=${LOCAD}/Programs
	PATH:=${PATH}:${PGF86}/Inno Setup 6
	PATH:=${PATH}:${PGF}/Inkscape/bin
	PATH:=${PATH}:${PGU}/Inkscape/bin
	PATH:=${PATH}:${PGF86}/Pandoc
	MAGICK=magick
else
	ECHO=echo
endif

mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
mkfile_dir := $(patsubst %/,%,$(dir $(mkfile_path)))
#current_dir := $(notdir $(mkfile_dir))

RC=windres
STRIP=strip
UPX=upx

VERSION=$(shell git describe --abbrev=0 --tags 2>/dev/null || echo 'Unknown_version')
COMMIT=$(shell git rev-parse --short HEAD 2>/dev/null || echo 'Unknown_commit')
DECORATION=Nawak-Bidon
ISO8601 := $(shell date +%Y-%m-%dT%H:%M:%SZ)
TMSTAMP := $(shell date +%Y%m%d%H%M%S)

#WVDIR=${mkfile_dir}/webview
#WV2SUBDIR=Microsoft.Web.WebView2.1.0.1150.38

CPPFLAGS += -I ${mkfile_dir}
#CPPFLAGS += -I${WVDIR}/build/external/libs/${WV2SUBDIR}/build/native/include

CXXFLAGS += -std=c++20 -O -s #-g
CXXFLAGS += -Wall # -pedantic -Wextra # Utiliser ces 2 dernières options de temps en temps peut-être utile ...
CXXFLAGS += -Wno-unknown-pragmas
LDFLAGS += -O -s #-g

ifeq (${OS},Linux)
#WK2GV=4.1
WK2GV=4.0
CXXFLAGS += $(shell pkg-config --cflags gtk+-3.0 webkit2gtk-${WK2GV})
LDFLAGS +=-L/usr/lib/x86_64-linux-gnu -L/usr/lib/x86_64-linux-gnu/webkit2gtk-${WK2GV} -L/usr/lib/x86_64-linux-gnu/cmake/harfbuzz -L/usr/lib/python3/dist-packages/cairo -L/usr/lib/x86_64-linux-gnu/glib-2.0 -L/usr/lib/x86_64-linux-gnu/glib-2.0
LDLIBS += $(shell pkg-config --libs gtk+-3.0 webkit2gtk-${WK2GV} webkit2gtk-web-extension-${WK2GV})
ifneq (${WITH_CURL},)
LDLIBS += $(shell pkg-config --libs libcurl)
endif
#LDFLAGS += -static
PANDOC=pandoc
else
EXEXT=.exe
CPPFLAGS += --include=${mkfile_dir}/webview_mingw_support.h
LDFLAGS += -mwindows
LDFLAGS += -static

ifneq (${WITH_CURL},)
#pacman -S mingw-w64-x86_64-curl-gnutls
ifeq ($(STATIC_CURL),1)
LDLIBS += -lcurl
else
LDLIBS += -lcurl.dll
endif
LDLIBS += -lidn2 -lssh2 -lpsl -lbcrypt -lcrypt32 -lzstd -lz -liconv -lunistring -lbrotlidec -lbrotlicommon
endif

LDLIBS += -lwldap32 -lws2_32 -ladvapi32 -lole32 -lshell32 -lshlwapi -loleaut32 -lcomctl32 -lactiveds -luser32 -lnetapi32 -lversion	
endif

#MSBUILD='C:\Program\ Files\Microsoft\ Visual\ Studio\2022\Community\MSBuild\Current\Bin\amd64\MSBuild.exe'
MSBUILD=/c/Program\ Files/Microsoft\ Visual\ Studio/2022/Community/MSBuild/Current/Bin/amd64/MSBuild.exe
DO_MSBUILD=$(shell test -f $(MSBUILD) && echo 1 || echo 0)
ifeq ($(MAKECMDGOALS),gcc)
DO_MSBUILD=0
endif

ifeq ($(DO_MSBUILD),1)
# These 2 environment variables cause troubles to the Visual Studio build
unexport TMP TEMP
endif

