
PATH:=${PATH}:/c/Program Files/Inkscape/bin

HTMINIFY=/c/Users/casal/AppData/Roaming/npm/html-minifier --collapse-whitespace --remove-comments --remove-optional-tags --remove-redundant-attributes --remove-script-type-attributes --remove-tag-whitespace --use-short-doctype --minify-css true --minify-js true --continue-on-parse-error

COMPONENTS=index.html head.js loaders.css style.css
TARGET=index.min.html

${TARGET} : app.ico ${COMPONENT}

app.ico : confluence.ico
	mv $< $@ 

clean :
	rm -f index.nomin.html ${TARGET} ${TARGET:hta=debug.html}


%.min.html : %.html
	@echo "PREPROCESSING HTML $<"
	@rm -f ${<:.html=.nomin.html}
	@./htpp.sh $< ${<:.html=.nomin.html}
	@echo "Checking ${<:.html=.nomin.html}"
	@tidy ${<:.html=.nomin.html} || true
#	@echo "Embedding app icon"
#	@sed -e 's/ icon=".*\.ico" / icon="\#" /i' ${<:.html=.nomin.html} | ${HTMINIFY} >${<:.html=.nomin.html}.tmp || true
#	@ cat app.ico ${<:.html=.nomin.html}.tmp >${<:.html=.nomin.html}  && rm ${<:.html=.nomin.html}.tmp
	@echo "MINIFYING HTML $<"
	@${HTMINIFY} ${<:.html=.nomin.html} > $@ || true
#	@rm -f ${<:.html=.nomin.html}

%.min.html : %.nomin.html
	@echo "MINIFYING HTML $<"
	@${HTMINIFY} $< > $@ || true

# Utile pour le debug
%.nomin.html : %.html
	@echo "PREPROCESSING HTML $<"
	@rm -f $@
	@./htpp.sh $< $@
	@echo "Checking $@"
	@tidy $@ || true

%.min.js : %.js
	@echo "MINIFYING JS $<"
	@rm -f $@
	@${JSMINIFY} $< > $@

%.ico : %.png
	${MAGICK} convert -background none $< $@

%.ico : %.svg
	${MAGICK} convert -background none +dither -colors 16 -depth 4 $< $@

