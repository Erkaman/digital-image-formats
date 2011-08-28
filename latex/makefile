TARGET=project
TARGETFILES = $(wildcard *.tex) $(wildcard tikz_img/*.tex)

all: ${TARGET}.pdf

${TARGET}.bbl: ${TARGET}.bib
	pdflatex ${TARGET}.tex
	bibtex ${TARGET}
	@pdflatex ${TARGET}.tex

${TARGET}.pdf: ${TARGET}.bbl ${TARGETFILES}
	@pdflatex ${TARGET}.tex
	makeindex ${TARGET}.idx
	@pdflatex ${TARGET}.tex

clean:
	rm -f ${TARGET}.{log,aux,bbl,blg,log,pdf,idx,out,toc,ind,ilg,run.xml}
	rm -f latex.exe.stackdump
	rm -f ${TARGET}-blx.bib
