MD_FILE = article.md
PDF_FILE = article.pdf
STYLE_DIR = ./style
BIB_FILE = $(STYLE_DIR)/sample.bib
CSL_FILE = $(STYLE_DIR)/ieee.csl
LATEX_CLASS = sigchi
TEXINPUTS := $(STYLE_DIR):$(TEXINPUTS)  # Ensures LaTeX searches style/

PANDOC_FLAGS = --pdf-engine=xelatex --citeproc \
  --metadata documentclass=$(LATEX_CLASS) \
  --metadata title="A Study on Pandoc and CHI LaTeX Class" \
  --metadata author="Love Arreborn, Nadim Lakrouz"

all: $(PDF_FILE)

$(PDF_FILE): $(MD_FILE) $(BIB_FILE) $(CSL_FILE)
	TEXINPUTS=$(TEXINPUTS) pandoc $(MD_FILE) -o $(PDF_FILE) $(PANDOC_FLAGS) \
	  --bibliography=$(BIB_FILE) \
	  --csl=$(CSL_FILE)

clean:
	rm -f $(PDF_FILE) *.aux *.log *.out *.bbl *.blg *.bcf *.run.xml

.PHONY: all clean
