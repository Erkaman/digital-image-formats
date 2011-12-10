TARGET=project
TARGETFILES = $(wildcard *.tex) $(wildcard tikz_img/*.tex)

all: ${TARGET}.pdf

${TARGET}.bcf: ${TARGET}.bib
	pdflatex ${TARGET}.tex
	biber ${TARGET}
#	@pdflatex ${TARGET}.tex

${TARGET}.pdf: ${TARGET}.bcf ${TARGETFILES}
	@pdflatex ${TARGET}.tex
	makeindex ${TARGET}.idx
	@pdflatex ${TARGET}.tex

clean:
	rm -f ${TARGET}.{bbl,bcf,blg,log,pdf,idx,out,toc,ind,ilg,run.xml,loa,lof,lot}
	rm -f *.log
	rm -f *.aux
	rm -f *.stackdump
