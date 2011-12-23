TARGET=project
TARGETFILES = $(wildcard *.tex) $(wildcard tikz_img/*.tex)

all: ${TARGET}.pdf

${TARGET}.bcf: ${TARGET}.bib
	pdflatex ${TARGET}.tex
	biber ${TARGET}

${TARGET}.pdf: ${TARGET}.bcf ${TARGETFILES}
	@pdflatex ${TARGET}.tex
	makeindex ${TARGET}.idx
	@pdflatex ${TARGET}.tex

clean:
	rm -f *.bbl
	rm -f *.bcf
	rm -f *.blg
	rm -f *.log
	rm -f *.pdf
	rm -f *.idx
	rm -f *.idx
	rm -f *.log
	rm -f *.aux
	rm -f *.lof
	rm -f *.lot
	rm -f *.loa
	rm -f *.ilg
	rm -f *.ind
	rm -f *.xml
	rm -f *.out
	rm -f *.toc
	rm -f *.stackdump
