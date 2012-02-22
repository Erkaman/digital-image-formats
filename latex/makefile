TARGET=project
TARGETFILES = $(wildcard *.tex) $(wildcard tikz_img/*.tex)

all: ${TARGET}.pdf

${TARGET}.bbl: ${TARGET}.bib
	pdflatex ${TARGET}.tex
	bibtex ${TARGET}

${TARGET}.pdf: ${TARGETFILES} ${TARGET}.bbl
	while (pdflatex ${TARGET}.tex ; \
	grep -q "Rerun to get cross" ${TARGET}.log ) do true ; \
	done

clean:
	rm -f *.bbl
	rm -f *.blg
	rm -f *.log
	rm -f *.pdf
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
	rm -f *-blx.bib