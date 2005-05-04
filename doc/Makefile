all: guides.all

# publish it all on the sourceforge web
publish:
	rsync -v -rlt -e ssh --cvs-exclude --delete --delete-excluded --modify-window=10 * plearner@plearn.sourceforge.net:/home/groups/p/pl/plearn/htdocs

guides.all: guides.ps guides.html guides.pdf 

doxygen:
	doxygen

guides.html: users_guide programmers_guide tools_guide

guides.pdf: users_guide.pdf programmers_guide.pdf tools_guide.pdf

guides.ps: users_guide.ps programmers_guide.ps tools_guide.ps




users_guide.all: users_guide users_guide.dvi users_guide.ps users_guide.pdf

users_guide: users_guide.tex
	latex2html -split 3 -show_section_numbers -local_icons users_guide.tex

users_guide.dvi: users_guide.tex
	latex users_guide
	latex users_guide
	latex users_guide

users_guide.ps: users_guide.dvi
	dvips -Pcmps -t letter users_guide -o

users_guide.pdf: users_guide.dvi
	pdflatex users_guide

programmers_guide.all: programmers_guide programmers_guide.dvi programmers_guide.ps programmers_guide.pdf

programmers_guide: programmers_guide.tex
	latex2html -split 3 -show_section_numbers -local_icons programmers_guide.tex

programmers_guide.dvi: programmers_guide.tex
	latex programmers_guide
	latex programmers_guide
	latex programmers_guide

programmers_guide.ps: programmers_guide.dvi
	dvips -Pcmps -t letter programmers_guide -o

programmers_guide.pdf: programmers_guide.dvi
	pdflatex programmers_guide




tools_guide.all: tools_guide tools_guide.dvi tools_guide.ps tools_guide.pdf

tools_guide: tools_guide.tex
	latex2html -split 3 -show_section_numbers -local_icons tools_guide.tex

tools_guide.dvi: tools_guide.tex
	latex tools_guide
	latex tools_guide

tools_guide.ps: tools_guide.dvi
	dvips -Pcmps -t letter tools_guide -o

tools_guide.pdf: tools_guide.tex
	pdflatex tools_guide






