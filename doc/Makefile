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

guides.all: installation_guide.all users_guide.all programmers_guide.all tools_guide.all faq.all

doxygen:
	doxygen

TEX_INTERACTION_MODE=nonstopmode
TEX_OPTIONS=--interaction $(TEX_INTERACTION_MODE)
PDFTEX_OPTIONS=-interaction=$(TEX_INTERACTION_MODE)

faq.all: faq faq.dvi faq.ps faq.pdf

faq: faq.tex
	latex2html -split 3 -show_section_numbers -local_icons faq.tex

faq.dvi: faq.tex
	latex $(TEX_OPTIONS) faq
	latex $(TEX_OPTIONS) faq
	latex $(TEX_OPTIONS) faq

faq.ps: faq.dvi
	dvips -Pcmps -t letter faq -o

faq.pdf: faq.dvi
	pdflatex $(PDFTEX_OPTIONS) faq

installation_guide.all: installation_guide installation_guide.dvi installation_guide.ps installation_guide.pdf

installation_guide: installation_guide.tex
	latex2html -split 3 -show_section_numbers -local_icons installation_guide.tex

installation_guide.dvi: installation_guide.tex
	latex $(TEX_OPTIONS) installation_guide
	latex $(TEX_OPTIONS) installation_guide
	latex $(TEX_OPTIONS) installation_guide

installation_guide.ps: installation_guide.dvi
	dvips -Pcmps -t letter installation_guide -o

installation_guide.pdf: installation_guide.dvi
	pdflatex $(PDFTEX_OPTIONS) installation_guide


users_guide.all: users_guide users_guide.dvi users_guide.ps users_guide.pdf

users_guide: users_guide.tex
	latex2html -split 3 -show_section_numbers -local_icons users_guide.tex

users_guide.dvi: users_guide.tex
	latex $(TEX_OPTIONS) users_guide
	latex $(TEX_OPTIONS) users_guide
	latex $(TEX_OPTIONS) users_guide

users_guide.ps: users_guide.dvi
	dvips -Pcmps -t letter users_guide -o

users_guide.pdf: users_guide.dvi
	pdflatex $(PDFTEX_OPTIONS) users_guide

programmers_guide.all: programmers_guide programmers_guide.dvi programmers_guide.ps programmers_guide.pdf

programmers_guide: programmers_guide.tex
	latex2html -split 3 -show_section_numbers -local_icons programmers_guide.tex

programmers_guide.dvi: programmers_guide.tex
	latex $(TEX_OPTIONS) programmers_guide
	latex $(TEX_OPTIONS) programmers_guide
	latex $(TEX_OPTIONS) programmers_guide

programmers_guide.ps: programmers_guide.dvi
	dvips -Pcmps -t letter programmers_guide -o

programmers_guide.pdf: programmers_guide.dvi
	pdflatex $(PDFTEX_OPTIONS) programmers_guide




tools_guide.all: tools_guide tools_guide.dvi tools_guide.ps tools_guide.pdf

tools_guide: tools_guide.tex
	latex2html -split 3 -show_section_numbers -local_icons tools_guide.tex

tools_guide.dvi: tools_guide.tex
	latex $(TEX_OPTIONS) tools_guide
	latex $(TEX_OPTIONS) tools_guide

tools_guide.ps: tools_guide.dvi
	dvips -Pcmps -t letter tools_guide -o

tools_guide.pdf: tools_guide.tex
	pdflatex $(PDFTEX_OPTIONS) tools_guide


api.html:
	pldoc        
