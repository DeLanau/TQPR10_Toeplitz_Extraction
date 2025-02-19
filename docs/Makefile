MD_FILE = article.md
BODY_TEX = article-body.tex
FINAL_TEX = style/article-final.tex
PDF_FILE = article.pdf
PREAMBLE = style/preamble.tex
ENDING = style/ending.tex
BIB_FILE = style/sample.bib
CSL_FILE = style/ieee.csl

export TEXINPUTS=.:./style//

.PHONY: all clean

all: $(PDF_FILE)

$(BODY_TEX): $(MD_FILE)
	# Convert markdown to LaTeX body
	pandoc $(MD_FILE) -o $(BODY_TEX) --bibliography=$(BIB_FILE) --csl=$(CSL_FILE) --wrap=none
	# Fix citations in the LaTeX file
	python fix_citations.py $(BODY_TEX)

$(FINAL_TEX): $(PREAMBLE) $(BODY_TEX) $(ENDING)
	# Move article-body.tex into style/ and concatenate with preamble and ending
	mv $(BODY_TEX) style/
	cat $(PREAMBLE) style/$(BODY_TEX) $(ENDING) > $(FINAL_TEX)

$(PDF_FILE): $(FINAL_TEX)
	echo 'Pre-processing done -- cd into style/ and run xelatex article-final, bibtex article-final then xelatex article-final twice'	

clean:
	rm -f style/$(BODY_TEX) $(FINAL_TEX) $(PDF_FILE) *.aux *.log *.out *.bbl *.blg *.bcf *.run.xml
