all: guides.all api.html

# publish it all on the sourceforge web
#publish:
#	rsync -v -rlt -e ssh --cvs-exclude --delete --delete-excluded --modify-window=10 * plearner@plearn.sourceforge.net:/home/groups/p/pl/plearn/htdocs

#publish:
#	scp -r * plearner@shell.berlios.de:/home/groups/plearn/htdocs/

publish: publish_guides
	scp -r html_autodoc python_modules_html plearner@shell.berlios.de:/home/groups/plearn/htdocs/

publish_guides:
	scp -r *.html *_guide faq *.ps *.pdf *.txt *.jpg plearner@shell.berlios.de:/home/groups/plearn/htdocs/

guides.all: installation_guide.all users_guide.all programmers_guide.all tools_guide.all faq.all machine_learning.all

autodoc_plearn_curses: 
	cd html_autodoc; plearn_curses htmlhelp html_config.plearn
	cd html_autodoc/doc; ln -s ../plearn.css .; cp ../plearn.jpg .;	cp ../background.png .

doxygen:
	doxygen Doxyfile3

doxygen-full:
	doxygen Doxyfile

doxygen-no-dot:
	doxygen Doxyfile2

doxygen-no-source:
	doxygen Doxyfile3

TMPDIR=tmp/
TEX_INTERACTION_MODE=nonstopmode
TEX_OPTIONS=--interaction $(TEX_INTERACTION_MODE) -halt-on-error -output-directory=$(TMPDIR)
PDFTEX_OPTIONS=-interaction=$(TEX_INTERACTION_MODE) -halt-on-error -output-directory=$(TMPDIR)

faq.all: faq faq.dvi faq.ps faq.pdf

faq: faq.tex
	latex2html -split 3 -show_section_numbers -local_icons faq.tex

faq.dvi: faq.tex
	mkdir -p $(TMPDIR)
	latex $(TEX_OPTIONS) faq
	latex $(TEX_OPTIONS) faq
	latex $(TEX_OPTIONS) faq
	cp $(TMPDIR)$@ .

faq.ps: faq.dvi
	dvips -Pcmps -t letter $(TMPDIR)faq -o

faq.pdf: faq.dvi
	pdflatex $(PDFTEX_OPTIONS) faq
	cp $(TMPDIR)$@ .

installation_guide.all: installation_guide installation_guide.dvi installation_guide.ps installation_guide.pdf

installation_guide: installation_guide.tex
	latex2html -split 3 -show_section_numbers -local_icons installation_guide.tex

installation_guide.dvi: installation_guide.tex
	mkdir -p $(TMPDIR)
	latex $(TEX_OPTIONS) installation_guide
	latex $(TEX_OPTIONS) installation_guide
	latex $(TEX_OPTIONS) installation_guide
	cp $(TMPDIR)$@ .

installation_guide.ps: installation_guide.dvi
	dvips -Pcmps -t letter $(TMPDIR)installation_guide -o

installation_guide.pdf: installation_guide.dvi
	pdflatex $(PDFTEX_OPTIONS) installation_guide.tex
	cp $(TMPDIR)$@ .


users_guide.all: users_guide users_guide.dvi users_guide.ps users_guide.pdf

users_guide: users_guide.tex
	latex2html -split 3 -show_section_numbers -local_icons users_guide.tex

users_guide.dvi: users_guide.tex
	mkdir -p $(TMPDIR)
	latex $(TEX_OPTIONS) users_guide
	latex $(TEX_OPTIONS) users_guide
	latex $(TEX_OPTIONS) users_guide
	cp $(TMPDIR)$@ .

users_guide.ps: users_guide.dvi
	dvips -Pcmps -t letter $(TMPDIR)users_guide -o

users_guide.pdf: users_guide.dvi
	pdflatex $(PDFTEX_OPTIONS) users_guide
	cp $(TMPDIR)$@ .

programmers_guide.all: programmers_guide programmers_guide.dvi programmers_guide.ps programmers_guide.pdf

programmers_guide: programmers_guide.tex
	latex2html -split 3 -show_section_numbers -local_icons programmers_guide.tex

programmers_guide.dvi: programmers_guide.tex
	mkdir -p $(TMPDIR)
	latex $(TEX_OPTIONS) programmers_guide
	latex $(TEX_OPTIONS) programmers_guide
	latex $(TEX_OPTIONS) programmers_guide
	cp $(TMPDIR)$@ .

programmers_guide.ps: programmers_guide.dvi
	dvips -Pcmps -t letter $(TMPDIR)programmers_guide -o

programmers_guide.pdf: programmers_guide.dvi
	pdflatex $(PDFTEX_OPTIONS) programmers_guide
	cp $(TMPDIR)$@ .


machine_learning.all: machine_learning machine_learning.dvi machine_learning.ps machine_learning.pdf

machine_learning: machine_learning.tex
	latex2html -split 3 -show_section_numbers -local_icons machine_learning.tex

machine_learning.dvi: machine_learning.tex
	mkdir -p $(TMPDIR)
	latex $(TEX_OPTIONS) machine_learning
	latex $(TEX_OPTIONS) machine_learning
	latex $(TEX_OPTIONS) machine_learning
	cp $(TMPDIR)$@ .

machine_learning.ps: machine_learning.dvi
	dvips -Pcmps -t letter $(TMPDIR)machine_learning -o

machine_learning.pdf: machine_learning.dvi
	pdflatex $(PDFTEX_OPTIONS) machine_learning
	cp $(TMPDIR)$@ .



tools_guide.all: tools_guide tools_guide.dvi tools_guide.ps tools_guide.pdf

tools_guide: tools_guide.tex
	latex2html -split 3 -show_section_numbers -local_icons tools_guide.tex

tools_guide.dvi: tools_guide.tex
	mkdir -p $(TMPDIR)
	latex $(TEX_OPTIONS) tools_guide
	latex $(TEX_OPTIONS) tools_guide
	cp $(TMPDIR)$@ .

tools_guide.ps: tools_guide.dvi
	dvips -Pcmps -t letter $(TMPDIR)tools_guide -o

tools_guide.pdf: tools_guide.tex
	pdflatex $(PDFTEX_OPTIONS) tools_guide
	cp $(TMPDIR)$@ .

tutonly.all: tutonly tutonly.dvi tutonly.ps tutonly.pdf

tutonly: tutonly.tex
	latex2html -split 3 -show_section_numbers -local_icons tutonly.tex

tutonly.dvi: tutonly.tex
	mkdir -p $(TMPDIR)
	latex $(TEX_OPTIONS) tutonly
	latex $(TEX_OPTIONS) tutonly
	cp $(TMPDIR)$@ .

tutonly.ps: tutonly.dvi
	dvips -Pcmps -t letter $(TMPDIR)tutonly -o

tutonly.pdf: tutonly.tex
	pdflatex $(PDFTEX_OPTIONS) tutonly
	cp $(TMPDIR)$@ .

api.html:
	pldoc        

clean:
	rm -rf *.aux *.dvi *.ps *.log *.out *.toc *.backup *~ tmp/*
	rm -rf faq installation_guide machine_learning tutonly tools_guide programmers_guide users_guide
	rm -rf faq.pdf installation_guide.pdf machine_learning.pdf 
	rm -rf tutonly.pdf tools_guide.pdf programmers_guide.pdf users_guide.pdf
