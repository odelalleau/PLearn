
// -*- C++ -*-

// TxtmatCommand.cc
//
// Copyright (C) 2003-2004 ApSTAT Technologies Inc. 
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
//  1. Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
// 
//  2. Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.
// 
//  3. The name of the authors may not be used to endorse or promote
//     products derived from this software without specific prior written
//     permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
// NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// This file is part of the PLearn library. For more information on the PLearn
// library, go to the PLearn Web site at www.plearn.org

// Authors: Pascal Vincent

/* *******************************************************      
 * $Id$ 
 ******************************************************* */

/*! \file TxtmatCommand.cc */
#include "TxtmatCommand.h"
#include <plearn/base/stringutils.h>
#include <plearn/base/ProgressBar.h>
#include <curses.h>

namespace PLearn {
using namespace std;

//! This allows to register the 'TxtmatCommand' command in the command registry
PLearnCommandRegistry TxtmatCommand::reg_(new TxtmatCommand);

TxtmatCommand::TxtmatCommand() :
    PLearnCommand("txtmat",

                  "Examination of text matrices (.txtmat files containing a TextFilesVMatrix object)",

                  "Usage: txtmat info <file.txtmat> \n"
                  "       prints basic info: length, width, ...\n"
                  "   or: txtmat fields <file.txtmat> \n"
                  "       prints the list of fields \n"
                  "   or: txtmat lines <file.txtmat> [<start_row>] [<nrows>] \n"
                  "       prints the raw lines (raw meaning not even split in fields) \n"
                  "   or: txtmat view <file.txtmat> \n"
                  "       displays the txtmat as text fields using curses \n" 
                  "   or: txtmat row <file.txtmat> <rownum> [+] \n"
                  "       prints all the fields of the given row. \n"
                  "       If + is specified, it will also print their conversion to real \n"
                  "   or: txtmat col <file.txtmat> <colnum or fieldname> [<start_row> <nrows>] [+]\n"
                  "       prints the text content of the specified column or field. \n"
                  "       If + is specified, it will also print their conversion to real \n"
                  "   or: txtmat scan <file.txtmat> \n"
                  "       Will call getRow on every row, and then save the string mappings. \n"
                  "       This insures that all is checked and mappings are built if needed. \n"
                  "   or: txtmat counts <file.txtmat> \n"
                  "       Will generate a counts/ directory in the metadatadir and record, \n"
                  "       for each field with a mapping, how many times a particular mapped string was encountered \n"
                  "   or: txtmat savemap <file.txtmat> \n"
                  "       Will generate and save standard VMatrix string mapping from the txtmat \n"
                  "       specific mapping. \n"
                  "   or: txtmat checkstuff <file.txtmat> Ex. de petite fonction pour Charles\n"
        ) 
{}

void TxtmatCommand::checkstuff(PP<TextFilesVMatrix> vm)
{
    int l = vm->length();
    TVec< pair<string, string> > fieldspec = vm->fieldspec;
    
    int NOPOL = vm->getIndexOfTextField("NOPOL");
    int NOVEH = vm->getIndexOfTextField("NOVEH");
    int DTEFPO = vm->getIndexOfTextField("DTEFPO");
    int UNTA1997 = vm->getIndexOfTextField("UNTA1997");
    int UNTA1998 = vm->getIndexOfTextField("UNTA1998");
    int UNTA1999 = vm->getIndexOfTextField("UNTA1999");
    int UNTA2000 = vm->getIndexOfTextField("UNTA2000");
    int UNTA2001 = vm->getIndexOfTextField("UNTA2001");

    double sum_1997 = 0;
    double sum_1998 = 0;
    double sum_1999 = 0;
    double sum_2000 = 0;
    double sum_2001 = 0;

    TVec<string> v = vm->getTextFields(0);
    string prev_NOPOL = v[NOPOL];
    string prev_NOVEH = v[NOVEH];
    string prev_DTEFPO = v[DTEFPO];

    for(int i=0; i<l; i++)
    {
        v = vm->getTextFields(i);
        if(prev_NOPOL!=v[NOPOL] || prev_NOVEH!=v[NOVEH] || prev_DTEFPO!=v[DTEFPO] || i==l-1)
        {
            cout << prev_NOPOL << " " << prev_NOVEH << " " << prev_DTEFPO << " \t" 
                 << sum_1997 << " " << sum_1998 << " " << sum_1999 << " " << sum_2000 << " " << sum_2001 << " \t"
                 << sum_1997+sum_1998+sum_1999+sum_2000+sum_2001 << endl;
            sum_1997 = sum_1998 = sum_1999 = sum_2000 = sum_2001 = 0;
            prev_NOPOL = v[NOPOL];
            prev_NOVEH = v[NOVEH];
            prev_DTEFPO = v[DTEFPO];
        }
        else
        {
            if(v[UNTA1997]!="")
                sum_1997 += atof(v[UNTA1997].c_str());
            if(v[UNTA1998]!="")
                sum_1998 += atof(v[UNTA1998].c_str());
            if(v[UNTA1999]!="")
                sum_1999 += atof(v[UNTA1999].c_str());
            if(v[UNTA2000]!="")
                sum_2000 += atof(v[UNTA2000].c_str());
            if(v[UNTA2001]!="")
                sum_2001 += atof(v[UNTA2001].c_str());
        }
    }
}

void TxtmatCommand::view(PP<TextFilesVMatrix> vm, int lin, int col)
{
    int nfields = vm->fieldspec.size();
    int l = vm->length();

    initscr();
    cbreak();
    noecho();
    keypad(stdscr,TRUE);
  
    int key= 0, curl= 0, curc= 0;
    bool view_vec = false;
  
    while(key != 'q' && key != 'Q')
    {
        erase();

        for(int j= 0; (j+2)*10 <= COLS && j+col<nfields; ++j)
        {
            string s= vm->fieldspec[j+col].first;
            mvprintw(0,(j+1)*10-1," %9s", s.substr(0,9).c_str());
            if(s.length() > 9)
                mvprintw(1,(j+1)*10-1," %9s", s.substr(9,9).c_str());
        }
        for(int i= 0; i < LINES-3 && i+lin<l; ++i)
        {
            TVec<string> fields = vm->getTextFields(i+lin);

            mvprintw(i+2,0,"%d", i+lin);
            for(int j= 0; (j+2)*10 <= COLS && j+col<nfields; ++j)
            {
                if(i == curl || j == curc)
                    attron(A_REVERSE);

                // real x= vm(i+lin, j+col);
                string s = fields[j+col];
                mvprintw(i+2,(j+1)*10-1," %9s", s.substr(0,9).c_str());

                attroff(A_REVERSE);
            }
        }

        //real x= vm(curl+lin, curc+col);
        //string strval= vm->getValString(curc+col, x);
        //if(!view_vec || strval == "")
        //strval= tostring(x);

        int fieldnum = curc+col;
        string strval = vm->getTextFields(curl+lin)[fieldnum];
        string fieldname = vm->fieldspec[fieldnum].first;
        string fieldtype = vm->fieldspec[fieldnum].second;
        pair<int,int> ra = vm->colrange[fieldnum];
        string transformed;
        if(view_vec)
        {
            try 
            { 
                if(ra.second>0)
                {
                    Vec dest(ra.second);
                    vm->transformStringToValue(fieldnum, strval, dest); 
                    for(int j=0; j<dest.length(); j++)
                    {
                        if(is_missing(dest[j]))
                            transformed += "MISSING ";
                        else
                            transformed += tostring(dest[j]) + " ";
                    }
                }
            }
            catch(const PLearnError& e)
            {
                mvprintw(1,1," %s", e.message().c_str());
                transformed = "ERROR! See message at top of screen.";
            }
        }
        else
            transformed = "[ Press n to view as reals ]";

        mvprintw(LINES-1,0," %dx%d  (%d, %d) [ %s : %s ]  \"%s\" --> %s", 
                 l, nfields,
                 curl+lin, fieldnum, fieldname.c_str(), fieldtype.c_str(), strval.c_str(), transformed.c_str());

        refresh();
        key= getch();

        switch(key)
        {
        case KEY_UP: 
            if(0 < curl) --curl; 
            else if(lin>0) --lin; 
            break;
        case KEY_DOWN: 
            if(curl < LINES-4 && curl+lin < l-1) ++curl; 
            else if(lin  < l-1) ++lin; 
            if(curl+lin >= l) curl= l-lin-1;
            break;
        case KEY_PPAGE: 
            lin-=LINES-3; 
            if(lin < 0) lin= 0; 
            break;
        case KEY_NPAGE: 
            lin+=LINES-3; 
            if(lin >= l) lin= l-1; 
            if(curl+lin >= l) curl= l-lin-1;
            break;
        case KEY_LEFT: 
            if(0 < curc) --curc; 
            else if(col>0) --col; 
            break;
        case KEY_RIGHT: 
            if(curc < COLS/10-2 && curc+col < nfields-1) ++curc; 
            else if(col < nfields-1) ++col;
            if(curc+col >= nfields) curc= nfields-col-1;
            break;
        case (int)'l': case (int)'L': 
        {
            echo();
            mvprintw(LINES-1,0,"Goto line:                                                                            ");
            move(LINES-1, 11);
            char li[10];
            getnstr(li, 10);
            if(!pl_isnumber(li) || toint(li) < 0 || toint(li)>=l)
            {
                mvprintw(LINES-1,0,"*** Invalid line number ***");
                refresh();
                sleep(1);
            }
            else
                lin= toint(li);
            noecho();
        }
        break;
        case (int)'c': case (int)'C': 
        {
            echo();
            mvprintw(LINES-1,0,"Goto column:                                                                           ");
            move(LINES-1, 13);
            char c[10];
            getnstr(c, 10);
            if(!pl_isnumber(c) || toint(c) < 0 || toint(c)>=nfields)
            {
                mvprintw(LINES-1,0,"*** Invalid column number ***");
                refresh();
                sleep(1);
            }
            else
                col= toint(c);
            noecho();
        }
        break;

        case (int)'s': case (int)'S': 
            if(!view_vec)
            {
                mvprintw(LINES-1,0,"*** Strings already shown ***");
                refresh();
                sleep(1);
            }
            else
                view_vec= false;
            break;

        case (int)'n': case (int)'N': 
            if(view_vec)
            {
                mvprintw(LINES-1,0,"*** Numbers already shown ***");
                refresh();
                sleep(1);
            }
            else
                view_vec= true;
            break;

        case (int)'h': case (int)'H':
            erase();

            mvprintw(0,COLS/2-6,"*** HELP ***");

            mvprintw(2,10,"KEYS:");
            mvprintw(3,10," - up: move up one line");
            mvprintw(4,10," - down: move down one line");
            mvprintw(5,10," - right: move right one column");
            mvprintw(6,10," - left: move left one column");
            mvprintw(7,10," - page up: move up one screen");
            mvprintw(8,10," - page down: move down one screen");
            mvprintw(9,10," - 'l' or 'L': prompt for a line number and go to that line");
            mvprintw(10,10," - 'c' or 'C': prompt for a column number and go to that column");
            mvprintw(11,10," - 's' or 'S': display string fields as strings");
            mvprintw(12,10," - 'n' or 'N': display string fields as numbers");
            mvprintw(13,10," - 'h' or 'H': display this screen");
            mvprintw(14,10," - 'q' or 'Q': quit program");

            mvprintw(16,COLS/2-13,"(press any key to continue)");

            refresh();
            getch();
	  
            break;

        case (int)'q': case (int)'Q': 
            break;
 
        default:
            mvprintw(LINES-1,0,"*** Invalid command (type 'h' for help) ***");
            refresh();
            sleep(1);
            break;
        }
    }
  
    endwin();
}

//! The actual implementation of the 'TxtmatCommand' command 
void TxtmatCommand::run(const vector<string>& args)
{
    int nargs = args.size();
    string com = args[0];
    string fname = args[1];
    PP<TextFilesVMatrix> m = dynamic_cast<TextFilesVMatrix*>(macroLoadObject(fname));

    if(!m)
        PLERROR("File %s does not appear to contain a TextFilesVMatrix object", fname.c_str());

    if(com=="info")
    {
        cout << "length: " << m->length() << endl;
        cout << "width: " << m->width() << endl;
    }
    else if(com=="fields")
    {
        int n = m->fieldspec.size();
        cout << n << " fields:" << endl;
        for(int j=0; j<n; j++)
            cout << j << ": \t" << m->fieldspec[j].first << "\t" << m->fieldspec[j].second << endl;
    }
    else if(com=="lines")
    {
        int startrow = 0;
        int l = 1;
        if(nargs>=3)
            startrow = toint(args[2]);
        if(nargs>=4)
            l = toint(args[3]);
        for(int i=startrow; i<startrow+l; i++)
            cout << i << ": \t" << m->getTextRow(i) << endl;
    }
    else if(com=="view")
    {
        view(m);
    }
    else if(com=="row")
    {
        int i = toint(args[2]);
        bool tonum = false;
        if(nargs>=4 && args[3]=="+")
            tonum = true;
        TVec<string> txtfields = m->getTextFields(i);
        Vec v;
        if(tonum)
        {
            v.resize(m->width());
            m->getRow(i,v);
        }
        int n = m->fieldspec.size();
        for(int j=0; j<n; j++)
        {
            cout << j << " " << m->fieldspec[j].first << ": \t";
            cout << txtfields[j] << "\t";
            if(tonum)
                cout << v[j];
            cout << endl;
        }
    }
    else if(com=="col")
    {
        string colname = args[2];
        int startrow = 0;
        int end = m->length();
        if(nargs>=4)
            startrow = toint(args[3]);
        if(nargs>=5)
            end = startrow+toint(args[4]);
        else
            end = 1;
        bool tonum = false;
        if(nargs>=6 && args[5]=="+")
            tonum = true;
        // find column with colname
        int col;
        for(col=0; col<m->fieldspec.size(); col++)
            if(m->fieldspec[col].first==colname)
                break;
        if(col>=m->fieldspec.size()) // not a valid name
            col = toint(colname);
        for(int i=startrow; i<end; i++)
        {
            string strval = m->getTextFields(i)[col];
            cout << i << ": \t\"" << strval << "\"\t";
            if(tonum)
            {
                string fieldtype = m->fieldspec[col].second;
                cout << "[" << fieldtype << "]";
                pair<int,int> ra = m->colrange[col];
                if(ra.second>0)
                {
                    cout << " --> ";
                    Vec dest(ra.second);
                    m->transformStringToValue(col, strval, dest); 
                    for(int j=0; j<dest.length(); j++)
                    {
                        if(is_missing(dest[j]))
                            cout << "MISSING_VALUE ";
                        else
                            cout << dest[j] << " ";
                    }
                }
            }
            cout << endl;
        }
    }
    else if(com=="scan")
    {
        int l = m->length();
        Vec v(m->width());
        ProgressBar pg("Scanning lines",l);
        for(int i=0; i<l; i++)
        {
            m->getRow(i,v);
            pg(i);
        }
    }
    else if(com=="counts")
    {
        m->generateMapCounts();
    }
    else if(com=="savemap")
    {
        m->buildVMatrixStringMapping();
        m->saveAllStringMappings();
    }
    else if(com=="checkstuff")
    {
        checkstuff(m);
    }
    else 
        PLERROR("Unknown command: txtmat %s", com.c_str());
}

} // end of namespace PLearn


/*
  Local Variables:
  mode:c++
  c-basic-offset:4
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:79
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=79 :
