<<<<<<< HEAD
TARGET=CS444Written1

pdf: ps
	ps2pdf ${TARGET}.ps

pdf-print: ps
	ps2pdf -dColorConversionStrategy=/LeaveColorUnchanged -dPDFSETTINGS=/printer ${TARGET}.ps

text: html
	html2text -width 100 -style pretty ${TARGET}/${TARGET}.html | sed -n '/./,$$p' | head -n-2 >${TARGET}.txt

html:
	@#latex2html -split +0 -info "" -no_navigation ${TARGET}
	htlatex ${TARGET}

ps:	dvi
	dvips -t letter ${TARGET}.dvi

dvi:
	latex ${TARGET}
	bibtex ${TARGET}||true
	latex ${TARGET}
	latex ${TARGET}

read:
	evince ${TARGET}.pdf &

aread:
	acroread ${TARGET}.pdf

clean:
=======
TARGET=CS444Written1

pdf: ps
	ps2pdf ${TARGET}.ps

pdf-print: ps
	ps2pdf -dColorConversionStrategy=/LeaveColorUnchanged -dPDFSETTINGS=/printer ${TARGET}.ps

text: html
	html2text -width 100 -style pretty ${TARGET}/${TARGET}.html | sed -n '/./,$$p' | head -n-2 >${TARGET}.txt

html:
	@#latex2html -split +0 -info "" -no_navigation ${TARGET}
	htlatex ${TARGET}

ps:	dvi
	dvips -t letter ${TARGET}.dvi

dvi:
	latex ${TARGET}
	bibtex ${TARGET}||true
	latex ${TARGET}
	latex ${TARGET}

read:
	evince ${TARGET}.pdf &

aread:
	acroread ${TARGET}.pdf

clean:
>>>>>>> 3ec81e9fcd2a837e62018712238b1ba478c84adc
	rm -f ${TARGET}.{ps,pdf,log,aux,out,dvi,bbl,blg}