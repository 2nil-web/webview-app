
ifneq ($(shell uname -s),Linux)
ECHO=echo -e
else
ECHO=echo
endif

PGF=$(subst \,/,$(subst C:\,/c/,$(PROGRAMFILES)))
PGF86=${PGF} (x86)
PATH:=${PATH}:${PGF86}/Inno Setup 6
PATH:=${PATH}:${PGF}/Inkscape/bin

MAGICK=magick
RC=windres
STRIP=strip
UPX=upx

VERSION=$(shell git describe --abbrev=0 --tags 2>/dev/null || echo 'Unknown_version')
COMMIT=$(shell git rev-parse --short HEAD 2>/dev/null || echo 'Unknown_commit')
DECORATION=Nawak-Bidon

WVDIR=webview-gcc
WV2DIR=Microsoft.Web.WebView2.1.0.1150.38
CPPFLAGS += -I${WVDIR} -I${WVDIR}/build/external/libs/${WV2DIR}/build/native/include --include=webview_mingw_support.h
CXXFLAGS += -std=c++20
CXXFLAGS += -Wall -pedantic # -Wextra
LDFLAGS += -static -mwindows
LDLIBS += -ladvapi32 -lole32 -lshell32 -lshlwapi -luser32 -lversion

MSBUILD='C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\amd64\MSBuild.exe'
DO_MSBUILD=$(shell test -f "${MSBUILD}" && echo 1 || echo 0)

ifeq ($(DO_MSBUILD),0)
	DO_MSBUILD=$(shell test -f $(cygpath "${MSBUILD}") && echo 1 || echo 0)
endif

EXEXT=.exe
PREFIX=webview-app
SRCS=$(wildcard *.cpp)
OBJS=$(SRCS:.cpp=.o)
OBJS += ${PREFIX}_res.o

TARGET=${PREFIX}${EXEXT}

.PHONY: FORCE

ifeq ($(DO_MSBUILD),1)
all : version_check.txt version.h ${PREFIX}.ico ${TARGET}
	${MSBUILD} webview-app.sln -p:Configuration=Release
	cp x64/Release/*.exe .
#	pandoc -o Summary.docx -f markdown -t docx Summary.md
else
all : version_check.txt version.h ${TARGET}
endif

${TARGET} : ${OBJS}

${PREFIX}_res.o : ${PREFIX}.ico


strip : $(TARGET)
	$(STRIP) $(TARGET) | true

upx : strip
	$(UPX) $(TARGET) | true

clean :
	rm -f *~ ${PREFIX}.ico *.o $(OBJS)

rclean :
	rm -f *~ *.d ${PREFIX}.ico *.o $(OBJS) $(TARGET) WebView2Loader.dll
	rm -rf x64

# Génération du version.h intégré dans l'appli
version.h : version_check.txt
	@${ECHO} "Building C++ header $@"
	@${ECHO} "std::string name=\"${PREFIX}\", version=\"${VERSION}\", decoration=\"${DECORATION}\", commit=\"${COMMIT}\", created_at=\"${ISO8601}\";" >$@

# Génération du version.json intégré dans le paquetage
version.json : version_check.txt
	@${ECHO} "Building json file $@"
	@${ECHO} '{ "name":"${PREFIX}", "version":"${VERSION}", "decoration":"${DECORATION}", "commit":"${COMMIT}","created_at":"${ISO8601}" }' >$@

# Pour regénérer version.h et version.json dès qu'un des champs version ou decoration ou commit, est modifié.
version_check.txt : FORCE
	@${ECHO} "Version:${VERSION}, decoration:${DECORATION}, commit:${COMMIT}" >new_$@
	@if diff new_$@ $@ >/dev/null 2>&1; then rm -f new_$@; else mv -f new_$@ $@; rm -f ${PREFIX}.iss ${PREFIX}-standalone.iss; fi

cfg :
	@echo "PGF ${PGF}"
	@echo "PGF86 ${PGF86}"
	@echo "PATH"
	@echo "${PATH}" | sed 's/:/\n/g'
	@echo "END PATH"
	which inkscape.exe

# Ces régles implicites ne sont pas utiles quand on fait 'make rclean' (voir même make clean ...)
ifneq ($(MAKECMDGOALS),rclean)
%.exe: %.o
	$(LINK.cpp) $^ $(LOADLIBES) $(LDLIBS) -o $@

%.exe: %.c
	$(LINK.c) $^ $(LOADLIBES) $(LDLIBS) -o $@

%.exe: %.cpp
	$(LINK.cc) $^ $(LOADLIBES) $(LDLIBS) -o $@

%.ico : %.png
	${MAGICK} convert -background none $< $@

%.ico : %.svg
	${MAGICK} convert -background none $< $@

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

