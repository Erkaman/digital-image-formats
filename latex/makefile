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
	rm -f ${TARGET}.{log,aux,bbl,bcf,blg,log,pdf,idx,out,toc,ind,ilg,run.xml,loa}
	rm -f *.log
	rm -f pdflatex.exe.stackdump
