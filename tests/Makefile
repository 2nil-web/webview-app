

# Intégrer clang-format
# js check + minify

# Bug d'affichage avec uname sous clang64 ...
ifeq (${MSYSTEM},CLANG64)
	UNAME=/usr/bin/uname
else
	UNAME=uname
endif

OS=$(shell ${UNAME} -s)

ifneq (${OS},Linux)
ECHO=echo -e
else
ECHO=echo
endif

PGF=$(subst \,/,$(subst C:\,/c/,$(PROGRAMFILES)))
PGF86=${PGF} (x86)
PATH:=${PATH}:${PGF86}/Inno Setup 6
PATH:=${PATH}:${PGF}/Inkscape/bin
PATH:=${PATH}:${PGF86}/Pandoc

ifneq (${OS},Linux)
 MAGICK=magick
endif
RC=windres
STRIP=strip
UPX=upx

VERSION=$(shell git describe --abbrev=0 --tags 2>/dev/null || echo 'Unknown_version')
COMMIT=$(shell git rev-parse --short HEAD 2>/dev/null || echo 'Unknown_commit')
DECORATION=Nawak-Bidon

WVDIR=../webview
WV2DIR=Microsoft.Web.WebView2.1.0.1150.38
CPPFLAGS += -I${WVDIR} -I${WVDIR}/build/external/libs/${WV2DIR}/build/native/include

CPPFLAGS+=-DCURL_STATICLIB


CXXFLAGS += -std=c++20 -g
CXXFLAGS += -Wall # -pedantic -Wextra # Utiliser ces 2 dernières options de temps en temps peut-être utile ...
LDFLAGS += -g

ifeq (${OS},Linux)
CXXFLAGS += $(shell pkg-config --cflags gtk+-3.0 webkit2gtk-4.1)
LDFLAGS +=-L/usr/lib/x86_64-linux-gnu -L/usr/lib/x86_64-linux-gnu/webkit2gtk-4.1 -L/usr/lib/x86_64-linux-gnu/cmake/harfbuzz -L/usr/lib/python3/dist-packages/cairo -L/usr/lib/x86_64-linux-gnu/glib-2.0 -L/usr/lib/x86_64-linux-gnu/glib-2.0
LDLIBS += $(shell pkg-config --libs gtk+-3.0 webkit2gtk-4.1 webkit2gtk-web-extension-4.1)
#LDFLAGS += -static
PANDOC=pandoc
else
ifeq (${MSYSTEM},CLANG64)
PATH:=/clang64/bin:$PATH
CC=clang
CXX=clang++
endif
EXEXT=.exe
CPPFLAGS += --include=webview_mingw_support.h
LDFLAGS += -mwindows
LDFLAGS += -static

LDLIBS += -ladvapi32 -lole32 -lshell32 -lshlwapi -luser32 -lversion #-lws2_32 -liconv -lunistring

#pacman -S mingw-w64-x86_64-curl-gnutls

LDLIBS += -Wl,-Bdynamic -lcurl -Wl,-Bstatic
LDLIBS +=  #-lssh2 -lpsl -lbcrypt -ladvapi32 -lcrypt32 -lwldap32 -lzstd -lbrotlidec -lz -lbrotlicommon -lidn2

#LDLIBS += -lcurl -lssh2 -lssh2 -lpsl -lbcrypt -ladvapi32 -lcrypt32 -lbcrypt -lwldap32 -lzstd -lzstd -lbrotlidec -lbrotlidec -lz -lws2_32
#LDLIBS += -lbrotlidec -lbrotlicommon -lidn2 -liconv -lunistring
endif

#MSBUILD='C:\Program\ Files\Microsoft\ Visual\ Studio\2022\Community\MSBuild\Current\Bin\amd64\MSBuild.exe'
MSBUILD=/c/Program\ Files/Microsoft\ Visual\ Studio/2022/Community/MSBuild/Current/Bin/amd64/MSBuild.exe
DO_MSBUILD=$(shell test -f $(MSBUILD) && echo 1 || echo 0)
ifeq ($(MAKECMDGOALS),gcc)
DO_MSBUILD=0
endif

ifeq ($(DO_MSBUILD),1)
ARCH=x64
CONF=Release
define genmsexe
	@sed 's/\<TemplateApp\>/$(1)/g' TemplateApp.sln >$1.sln
	@sed 's/\<TemplateApp\.cpp\>/$(1).cpp/g;s/\<TemplateApp\>/$(1)/g' TemplateApp.vcxproj >$1.vcxproj
	@${MSBUILD} $1.sln -p:Configuration=${CONF} >$1.mslog && mv ${ARCH}/${CONF}/$1.exe . && rm -f $1.mslog
	@rm -f $1.sln $1.vcxproj*
	@rm -rf ${ARCH}
endef
endif

SRCS=$(wildcard *.cpp)
SRCS:=$(filter-out TemplateApp.cpp, $(SRCS))
EXES=${SRCS:.cpp=.exe}


all : ${EXES}

gcc : all

strip :
	strip *.exe | true

upx : strip
	upx *.exe | true

format :
	@clang-format -style="{ BasedOnStyle: Microsoft, IndentWidth: 2 }" --sort-includes -i ${SRCS}

clean :
	rm -f ${EXES} *.mslog

# Ces régles implicites ne sont pas utiles quand on fait 'make clean'
ifneq ($(MAKECMDGOALS),clean)
%.docx : %.md
	pandoc -o $@ -f markdown -t docx $<

%.ico : %.png
	${MAGICK} convert -background none $< $@

%.ico : %.svg
	${MAGICK} convert -background none $< $@

%${EXEXT}: %.o
	$(LINK.cpp) $^ $(LOADLIBES) $(LDLIBS) -o $@

%${EXEXT}: %.c
	$(LINK.c) $^ $(LOADLIBES) $(LDLIBS) -o $@

%${EXEXT}: %.cpp
ifeq ($(DO_MSBUILD),1)
	@echo "Building $@ from $^ with msvc"
	$(call genmsexe,$*,${ARCH},${CONF})
else
	$(LINK.cc) $^ $(LOADLIBES) $(LDLIBS) -o $@

# Régles pour construire les fichier objet d'après les .rc
%.o : %.rc
	$(RC) $(CPPFLAGS) $< --include-dir . $(OUTPUT_OPTION)

%.d: %.c
	@echo Checking header dependencies from $<
	@$(COMPILE.c) -isystem /usr/include -MM $< >> $@

#	@echo "Building "$@" from "$<
%.d: %.cpp
	@echo Checking header dependencies from $<
	@$(COMPILE.cpp) -isystem /usr/include -MM $< >> $@

# Inclusion des fichiers de dépendance .d
ifdef OBJS
-include $(OBJS:.o=.d)
endif
endif
endif

