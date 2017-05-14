DOC=your_main_tex_file # without extension!
BIB=${DOC}

# default render options (also see .tex example):
RENDER=rendertwocol
OUTPUT_SUFFIX=_2col

LATEX_CODE=\\def\\${RENDER}{1} \\input{${DOC}}

TMP_DIR=tmp
LATEX=pdflatex
FLAGS=--output-directory=${TMP_DIR} --aux-directory=${TMP_DIR}


all: onecol twocol


_makepdf:
	mkdir ${TMP_DIR} -p
	${LATEX} "${LATEX_CODE}" ${FLAGS}
	cp ${DOC}.bib tmp
	cd tmp; bibtex ${BIB}
	rm tmp/${DOC}.bib
	${LATEX} "${LATEX_CODE}" ${FLAGS}
	${LATEX} "${LATEX_CODE}" ${FLAGS}
	mv tmp/${DOC}.pdf ${DOC}${OUTPUT_SUFFIX}.pdf

twocol:
	make RENDER=rendertwocol OUTPUT_SUFFIX=_2col _makepdf

onecol:
	make RENDER=renderonecol OUTPUT_SUFFIX=_1col _makepdf

clean:
	rm -rf ${TMP_DIR}