
# Génération du version.h intégré dans l'appli
version.h : version_check.txt
	@${ECHO} "Building C++ header $@"
	@${ECHO} "#ifndef VERSION_H\n#define VERSION_H\nnamespace app_info { std::string name=\"${PREFIX}\", version=\"${VERSION}\", decoration=\"${DECORATION}\", commit=\"${COMMIT}\", created_at=\"${ISO8601}\"; }\n#endif" >$@

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
	@which inkscape
	@echo "DO_MSBUILD [${DO_MSBUILD}]"
	@echo "TMP ${TMP}"
	@echo "TEMP ${TEMP}"
	@echo "tmp ${tmp}"
	@echo "temp ${temp}"
	@echo "root_dir ${root_dir}"
	@echo "mkfile_path ${mkfile_path}"
	@echo "mkfile_dir ${mkfile_dir}"
	@echo "current_dir ${current_dir}"

ALL_SRCS=$(wildcard *.cpp) $(wildcard *.hpp) $(wildcard *.h)
format :
	@echo "Formatting the following files: ${ALL_SRCS}"
	@clang-format -style="{ BasedOnStyle: Microsoft, IndentWidth: 2 }" --sort-includes -i ${ALL_SRCS}

# Ces régles implicites ne sont pas utiles quand on fait 'make clean'
ifneq ($(MAKECMDGOALS),clean)
ifeq ($(DO_MSBUILD),1)
ARCH=x64
CONF=Release
define genmsexe
	rm -rf ${ARCH}; \
	[ -f $1 ] || ( sed 's/\<TemplateApp\>/$(1)/g' TemplateApp.sln >${1}.sln;  sed 's/\<TemplateApp\.cpp\>/$(1).cpp/g;s/\<TemplateApp\>/$(1)/g' TemplateApp.vcxproj >${1}.vcxproj ); \
	${MSBUILD} $1.sln -p:Configuration=${CONF} >${1}.mslog && mv ${ARCH}/${CONF}/${1}.exe . && rm -f ${1}.mslog; \
	[ -f ${1}.exe ] && ( rm -f ${1}.sln ${1}.vcxproj*; rm -rf ${ARCH} )
endef
endif

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
	@echo "Building $@ for ${ARCH} ${CONF} from $^ with msvc"
	@$(call genmsexe,$*)
else
	@echo "Building $@ from $^ with g++"
	$(LINK.cc) $^ $(LOADLIBES) $(LDLIBS) -o $@
endif

ifneq ($(DO_MSBUILD),1)
# Régles pour construire les fichier objet d'après les .rc
%.o : %.rc
	$(RC) $< --include-dir . $(OUTPUT_OPTION)

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

