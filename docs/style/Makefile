TARGET = article-final

all: $(PDF_FILE)

$(PDF_FILE): $(TEX_FILE)
	xelatex article_final
	bibtex article_final
	xelatex article_final
	xelatex article_final
	mv article_final.pdf ..

clean:
	rm -f $(TARGET).aux $(TARGET).bbl $(TARGET).blg $(TARGET).log $(TARGET).out $(TARGET).pdf

.PHONY: all clean
