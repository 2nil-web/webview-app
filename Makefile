
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

WVDIR=webview
WV2DIR=Microsoft.Web.WebView2.1.0.1150.38
CPPFLAGS += -I${WVDIR} -I${WVDIR}/build/external/libs/${WV2DIR}/build/native/include --include=webview_mingw_support.h
CXXFLAGS += -std=c++20
CXXFLAGS += -Wall # -pedantic -Wextra # Utiliser ces 2 dernières options de temps en temps peut-être utile ...
LDFLAGS += -static -mwindows
LDLIBS += -ladvapi32 -lole32 -lshell32 -lshlwapi -luser32 -lversion

MSBUILD='C:\Program\ Files\Microsoft\ Visual\ Studio\2022\Community\MSBuild\Current\Bin\amd64\MSBuild.exe'
MSBUILD=/c/Program\ Files/Microsoft\ Visual\ Studio/2022/Community/MSBuild/Current/Bin/amd64/MSBuild.exe
#DO_MSBUILD=$(shell test -f $(MSBUILD) && echo 1 || echo 0)
DO_MSBUILD=1

EXEXT=.exe
PREFIX=webview-app
SRCS=$(wildcard *.cpp)
OBJS=$(SRCS:.cpp=.o)
OBJS += ${PREFIX}_res.o

TARGET=${PREFIX}${EXEXT}

.PHONY: FORCE

DO_MSBUILD=0
ifeq ($(DO_MSBUILD),1)
ARCH=x64
CONF=Release
all : version_check.txt version.h ${PREFIX}.ico ${TARGET}
#	pandoc -o README.docx -f markdown -t docx README.md

${TARGET} : ${ARCH}/${CONF}/${TARGET}

${ARCH}/${CONF}/${TARGET} : ${SRCS} ${PREFIX}_res.rc
	${MSBUILD} webview-app.sln -p:Configuration=${CONF}
	cp ${ARCH}/${CONF}/*.exe .
else
all : version_check.txt version.h ${TARGET}

${TARGET} : ${OBJS}
endif

${PREFIX}_res.o : ${PREFIX}.ico


strip : $(TARGET)
	$(STRIP) $(TARGET) | true

upx : strip
	$(UPX) $(TARGET) | true

clean :
	rm -f *~ *.d ${PREFIX}.ico *.o $(OBJS) $(TARGET)
ifeq ($(DO_MSBUILD),1)
	rm -rf ${ARCH}
endif

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
	@which inkscape.exe
	@echo "DO_MSBUILD [${DO_MSBUILD}]"

# Ces régles implicites ne sont pas utiles quand on fait 'make clean'
ifneq ($(MAKECMDGOALS),clean)
%.ico : %.png
	${MAGICK} convert -background none $< $@

%.ico : %.svg
	${MAGICK} convert -background none $< $@

ifneq ($(DO_MSBUILD),1)
%.exe: %.o
	$(LINK.cpp) $^ $(LOADLIBES) $(LDLIBS) -o $@

%.exe: %.c
	$(LINK.c) $^ $(LOADLIBES) $(LDLIBS) -o $@

%.exe: %.cpp
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

