PROJECT= writeup
TEX= pdflatex
BIBTEX= bibtex


all:
	$(TEX) $(PROJECT).tex >/dev/null
	gcc -o conc1 conc1.c -pthread
	rm -f *.log *.bak *.aux *.bbl *.blg *.idx *.toc *.out *~

clean-all:
	rm -f *.dvi *.log *.bak *.aux *.bbl *.blg *.idx *.ps *.eps *.pdf *.toc *.out *~

clean:
	rm -f *.log *.bak *.aux *.bbl *.blg *.idx *.toc *.out *~ conc1
