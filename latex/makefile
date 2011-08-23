TARGET=project
TARGETFILES = $(wildcard *.tex) $(wildcard tikz_img/*.tex)

all: ${TARGET}.pdf

%.pdf : %.dvi
	pdflatex $*

${TARGET}.bbl: ${TARGET}.bib
	latex ${TARGET}.tex
	makeindex ${TARGET}.idx
	bibtex ${TARGET}
	@latex ${TARGET}.tex

${TARGET}.dvi: ${TARGET}.bbl  ${TARGETFILES}
	@latex ${TARGET}.tex

ps: ${TARGET}.ps

${TARGET}.ps: ${TARGET}.dvi
	@dvips -t a4 ${TARGET}.dvi

clean:
	rm -f ${TARGET}.{log,aux,ps,dvi,bbl,blg,log,ps,pdf,idx,out,toc,ind,ilg,run.xml}
	rm -f latex.exe.stackdump
	rm -f ${TARGET}-blx.bib
