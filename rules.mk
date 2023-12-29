
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
	@echo "Building $@ from $^ with msvc"
	@$(call genmsexe,$*)
else
	@echo "Building $@ from $^ with g++"
	$(LINK.cc) $^ $(LOADLIBES) $(LDLIBS) -o $@
endif

ifneq ($(DO_MSBUILD),1)
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

