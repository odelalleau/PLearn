// -*- C++ -*-

// vmatmain.cc
// Copyright (C) 2002 Pascal Vincent, Julien Keable, Xavier Saint-Mleux, Rejean Ducharme
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

/* *******************************************************
 * $Id: vmatmain.cc 6316 2006-10-16 23:22:54Z lamblin $
 ******************************************************* */

#include "viewVMat.h"
#include <algorithm>                         // for max
#include <plearn/base/general.h>
#include <plearn/base/stringutils.h>
#include <plearn/vmat/ConcatColumnsVMatrix.h>
#include <plearn/vmat/SortRowsVMatrix.h>
#include <plearn/vmat/SubVMatrix.h>
#include <plearn/db/getDataSet.h>

#if defined(WIN32) && !defined(__CYGWIN__)
// There does not seem to be a Windows implementation of 'ncurses', thus we
// use 'pdcurses' instead.
#include <pdcurses/curses.h>
#else
#include "curses.h"
#endif

// Some of the above 'curses.h' includes may define those annoying macros,
// which would conflict with the code that follows.
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#ifdef clear
#undef clear
#endif

namespace PLearn {
using namespace std;


// returns false if the input is invalid and write in strReason the reason
bool getList(char* str, int curj, const VMat& vm, Vec& outList,
             char* strReason)
{
    vector<string>columnList;
    if (str[0] == '\0')
    {
        // nothing was inserted, then gets the current column
        char strj[10];
        sprintf(strj, "%d", curj);
        columnList.push_back(strj);
    }
    else
    {
        columnList = split(str, " -,", true);
    }

    vector<string>::iterator vsIt;

    // checks for errors
    bool invalidInput = false;
    int colVal = 0;
    char separator = 0;

    for (vsIt = columnList.begin(); vsIt != columnList.end(); vsIt++)
    {
        if (pl_isnumber(*vsIt))
        {
            if (colVal > toint(*vsIt) && separator == '-')
            {
                invalidInput = true;
                strcpy(strReason,
                       "Second element in range smaller than the first");
                break;
            }
            colVal = toint(*vsIt);
            if (colVal < 0 || colVal >= vm->width())
            {
                invalidInput = true;
                strcpy(strReason, "Invalid column number");
                break;
            }
        }
        else
        {
            // there was already a separator!
            if (separator == '-')
            {
                invalidInput = true;
                strcpy(strReason, "Too many '-' separators");
                break;
            }

            separator = (*vsIt)[0];
            if (separator != '-' &&
                separator != ',')
            {
                invalidInput = true;
                strcpy(strReason, "Invalid column separator");
                break;
            }
        }
    }

    outList.clear();
    if (separator == '-')
    {
        int start = toint(columnList.front());
        int end = toint(columnList.back());
        for (int colIdx = start; colIdx <= end; ++colIdx)
            outList.push_back(colIdx);
    }
    else if (separator == ',')
    {
        for (vsIt = columnList.begin(); vsIt != columnList.end(); ++vsIt)
        {
            if (pl_isnumber(*vsIt))
                outList.push_back(toint(*vsIt));
        }
    }
    else if (separator == 0)
    {
        outList.push_back(toint(columnList.front()));
    }

    return invalidInput;
}

void viewVMat(const VMat& vm, PPath filename)
{
    initscr();
    cbreak();
    noecho();
    keypad(stdscr,TRUE);

    VMat vm_showed = vm;

    int key = 0;
    bool view_strings = true;
    // If 'indent_strings_left' is set to false, then strings will be indented
    // to the right.
    bool indent_strings_left = true;
    //! Can take three values:
    //! 0 - usual display
    //! 1 - values that are *exactly* the same as the one of the previous vmat
    //!     line will be replaced by ...
    //! 2 - values that are *approximately* the same as the one of the
    //!     previous vmat line will be replaced by ...
    int hide_sameval = 0;
    bool transposed = false;

    //! if true we will display the filename at the bottom of the screan instead
    //! of the normal other information.
    bool display_filename = false;
    int namewidth = 0;
    for(int j=0; j<vm->width(); j++)
        namewidth = max(namewidth, (int) vm->fieldName(j).size());
    int namewidth_ = namewidth;

    int valwidth = 15;
    int valstrwidth = valwidth-1;

    const char* valstrformat = "%14s";

    int curi = 0;
    int curj = 0;
    int starti = 0;
    int startj = 0;

    int vStartHelp = 0;

    bool onError=false;

    map<int,Vec> cached_columns;

    try {

        while(key != 'q' && key != 'Q')
        {
            erase();

            int leftcolwidth = transposed ?1+namewidth :10;

            int nj = transposed ? LINES-3 : (COLS-leftcolwidth)/valwidth;
            int ni = transposed ? (COLS-leftcolwidth)/valwidth : LINES-4;
            if(display_filename && filename.length()>(size_t)COLS)
                ni -= filename.length()/COLS;

            int endj = min(vm_showed->width(), startj+nj);
            int endi = min(vm_showed->length(), starti+ni);

            int x=0, y=0; // (curses coordinates are (y,x) )

            // print field names
            for(int j=startj; j<endj; j++)
            {
                string s = vm_showed->fieldName(j);
                if ( int(s.size()) > namewidth)
                    s = s.substr(0, namewidth);

                // if(j==curj)
                //  attron(A_REVERSE);
                if(transposed)
                    mvprintw(1+(j-startj),0,"%s", s.c_str());
                else
                {
                    x = 1+leftcolwidth+(j-startj)*valwidth;
                    mvprintw(0, x, valstrformat,
                             s.substr(0,valstrwidth).c_str() );
                    if((int)s.length() > valstrwidth)
                        mvprintw(1, x, valstrformat,
                                 s.substr(valstrwidth,valstrwidth).c_str() );
                }
                // attroff(A_REVERSE);
            }

            Vec v(vm_showed.width());
            Vec oldv(vm_showed.width());

            for(int i=starti; i<endi; i++)
            {
                if(transposed)
                {
                    y = 0;
                    x = 1+leftcolwidth+(i-starti)*valwidth;
                    mvprintw(y,x,"%14d",i);
                }
                else
                {
                    y = i-starti+2;
                    x = 0;
                    mvprintw(y,x,"%d",i);
                }

                vm_showed->getRow(i,v);

                for(int j=startj; j<endj; j++)
                {
                    real val = v[j];
                    string s = vm_showed->getValString(j,val);
                    if (!view_strings || s == "")
                    {
                        // We only have 14 characters, and have to display
                        // correctly numbers like "18270326".
                        // Best compromise found is "%14.8g".
                        char tmp[1000];
                        sprintf(tmp, "%14.8g", val);
                        s = tmp;
                    }
                    else {
                        // This is a string. Maybe we want to indent it to the
                        // right.
                        // In this case we truncate it to its last characters.
                        if (!indent_strings_left) {
                            if (s.size() >= (size_t) valstrwidth) {
                                s = s.substr(s.size() - valstrwidth,
                                             valstrwidth);
                            } else {
                                string added_spaces((size_t) (valstrwidth - s.size()), ' ');
                                s = added_spaces + s;
                            }
                        }
                    }

                    if(transposed)
                        y = 1+(j-startj);
                    else
                        x = 1+leftcolwidth+(j-startj)*valwidth;

                    if( i == curi || (vm_showed.width() > 1 && j == curj) )
                        attron(A_REVERSE);
                    //else if ()
                    //  attron(A_REVERSE);

                    if(hide_sameval== 2 && i>starti && (is_equal(val,oldv[j])) )
                        mvprintw(y, x, valstrformat, "...");
                    else if(fast_exact_is_equal(hide_sameval, 1) && i>starti &&
                            (fast_exact_is_equal(val, oldv[j]) ||
                             (is_missing(val) && is_missing(oldv[j]))))
                        mvprintw(y, x, valstrformat, "...");
                    else
                        mvprintw(y, x, valstrformat,
                                 s.substr(0,valstrwidth).c_str());

                    attroff(A_REVERSE);
                }
                oldv << v;
            }

            string strval = vm_showed->getString(curi, curj);
            mvprintw(0,0,"Cols[%d-%d]", 0, vm_showed.width()-1);
            if(display_filename){
                mvprintw(LINES-filename.length()/COLS-1,0,"%s",filename.c_str());
            }else
                mvprintw(LINES-1,0,
                         " %dx%d   line= %d   col= %d     %s = %s (%f)",
                         vm_showed->length(), vm_showed->width(),
                         curi, curj, vm_showed->fieldName(curj).c_str(),
                         strval.c_str(), vm_showed(curi,curj));

            refresh();
            if (!onError)
                key = getch();
            else
                onError = false;

            ///////////////////////////////////////////////////////////////
            switch(key)
            {
            case KEY_LEFT:
                if(transposed)
                {
                    if(curi>0)
                        --curi;
                    if(curi<starti)
                        starti = curi;
                }
                else
                {
                    if(curj>0)
                        --curj;
                    if(curj<startj)
                        startj=curj;
                }
                break;
                //////////////////////////////////////////////////////////////
            case KEY_RIGHT:
                if(transposed)
                {
                    if(curi<vm_showed->length()-1)
                        ++curi;
                    if(curi>=starti+ni)
                        ++starti;
                }
                else
                {
                    if(curj<vm_showed->width()-1)
                        ++curj;
                    if(curj>=startj+nj)
                        ++startj;
                }
                break;
                //////////////////////////////////////////////////////////////
            case KEY_UP:
                if(transposed)
                {
                    if(curj>0)
                        --curj;
                    if(curj<startj)
                        startj=curj;
                }
                else
                {
                    if(curi>0)
                        --curi;
                    if(curi<starti)
                        starti = curi;
                }
                break;
                //////////////////////////////////////////////////////////////
            case KEY_DOWN:
                if(transposed)
                {
                    if(curj<vm_showed->width()-1)
                        ++curj;
                    if(curj>=startj+nj)
                        ++startj;
                }
                else
                {
                    if(curi<vm_showed->length()-1)
                        ++curi;
                    if(curi>=starti+ni)
                        ++starti;
                }
                break;
                //////////////////////////////////////////////////////////////
            case KEY_PPAGE:
                if(transposed)
                {
                    curj -= nj;
                    startj -= nj;
                    if(startj<0)
                        startj = 0;
                    if(curj<0)
                        curj = 0;
                }
                else
                {
                    curi -= ni;
                    starti -= ni;
                    if(starti<0)
                        starti = 0;
                    if(curi<0)
                        curi = 0;
                }
                break;
                //////////////////////////////////////////////////////////////
            case KEY_NPAGE:
                if(transposed)
                {
                    curj += nj;
                    startj += nj;
                    if(curj>=vm_showed->width())
                        curj = vm_showed->width()-1;
                    if(startj>vm_showed->width()-nj)
                        startj = max(0,vm_showed->width()-nj);
                }
                else
                {
                    curi += ni;
                    starti += ni;
                    if(curi>=vm_showed->length())
                        curi = vm_showed->length()-1;
                    if(starti>vm_showed->length()-ni)
                        starti = max(0,vm_showed->length()-ni);
                }
                break;
                //////////////////////////////////////////////////////////////
            case KEY_HOME:
                // not working on unix for the moment: see
                // http://dickey.his.com/xterm/xterm.faq.html#xterm_pc_style
                if(transposed)
                {
                    curi = 0;
                    starti = 0;
                }
                else
                {
                    curj = 0;
                    startj = 0;
                }
                break;
                //////////////////////////////////////////////////////////////
            case KEY_END:
                // not working on unix for the moment: see
                // http://dickey.his.com/xterm/xterm.faq.html#xterm_pc_style
                if(transposed)
                {
                    curi = vm_showed->length()-1;
                    starti = max(curi-ni + 1, 0);
                }
                else
                {
                    curj = vm_showed->width()-1;
                    startj = max(curj-nj + 1, 0);
                }
                break;
                //////////////////////////////////////////////////////////////
            case '.':
                if (hide_sameval == 1)
                    hide_sameval = 0;
                else
                    hide_sameval = 1;
                break;
                //////////////////////////////////////////////////////////////
            case ',':
                if (hide_sameval == 2)
                    hide_sameval = 0;
                else
                    hide_sameval = 2;
                break;
                //////////////////////////////////////////////////////////////
            case 't': case 'T':
                transposed = !transposed;
                nj = transposed ? LINES-3 : (COLS-leftcolwidth)/valwidth;
                ni = transposed ? (COLS-leftcolwidth)/valwidth : LINES-4;
                starti = max(0,curi-ni/2);
                startj = max(0,curj-nj/2);
                //endj = min(vm_showed->width(), startj+nj);
                //endi = min(vm_showed->length(), starti+ni);
                break;
                //////////////////////////////////////////////////////////////
            case '/':  // search for value
            {
                echo();
                char strmsg[] = {"Search for value or string: "};
                mvprintw(LINES-1,0,strmsg);
                // clear the rest of the line
                clrtoeol();
                move(LINES-1, (int)strlen(strmsg));
                char l[20];
                getnstr(l, 20);
                string searchme = removeblanks(l);
                real searchval = vm_showed(curi,curj);
                if(searchme!="")
                {
                    searchval = vm_showed->getStringVal(curj, searchme);
                    if(is_missing(searchval))
                    {
                        searchval = toreal(searchme);
                        // This one gives a very bad error: to be changed
                        if(is_missing(searchval))
                            PLERROR("Search item is neither a string with a valid mapping, nor convertible to a real");
                    }
                }

                Vec cached;
                if(cached_columns.find(curj)!=cached_columns.end())
                    cached = cached_columns[curj];
                else
                {
                    mvprintw(LINES-1,0,"Building cache...");
                    // clear the rest of the line
                    clrtoeol();
                    refresh();
                    cached.resize(vm_showed->length());
                    vm_showed->getColumn(curj,cached);
                    cached_columns[curj] = cached;
                }

                mvprintw(LINES-1,0,"Searching for value %f ...",searchval);
                clrtoeol();
                refresh();
                int curi_backup = curi;
                ++curi; // start searching from next row
                while(curi<vm_showed->length() &&
                      !fast_exact_is_equal(cached[curi], searchval))
                    ++curi;
                bool found = (curi < vm_showed->length());
                if (!found) {
                    // Try approximate match.
                    curi = curi_backup + 1;
                    while(curi<vm_showed->length() &&
                            !is_equal(cached[curi], searchval))
                        ++curi;
                }
                if(curi>=vm_showed->length())
                    curi = 0;
                ni = transposed ? (COLS-leftcolwidth)/valwidth : LINES-4;
                starti = max(0,curi-ni/2);
            }
            break;
            ///////////////////////////////////////////////////////////////
            case (int)'l': case (int)'L':
            {
                echo();
                char strmsg[] = {"Goto line: "};
                mvprintw(LINES-1,0,strmsg);
                clrtoeol();
                move(LINES-1, (int)strlen(strmsg));
                char l[10];
                getnstr(l, 10);
                if(l[0] == '\0' || !pl_isnumber(l) || toint(l) < 0 || toint(l)>=vm_showed->length())
                {
                    mvprintw(LINES-1,0,"*** Invalid line number ***");
                    clrtoeol();
                    refresh();
                    // wait until the user types something
                    key = getch();
                    onError = true;
                }
                else
                {
                    curi= toint(l);
                    starti = max(curi-ni + 1, 0);
                    //starti = curi;
                }
                noecho();
            }
            break;
            ///////////////////////////////////////////////////////////////
            case (int)'c': case (int)'C':
            {
                echo();
                char strmsg[] = {"Goto column: "};
                mvprintw(LINES-1,0,strmsg);
                clrtoeol();
                move(LINES-1, (int)strlen(strmsg));
                char c[200];
                getnstr(c, 10);
                string the_col = c;
                int col_num = -1;
                try {
                    col_num = vm_showed->getFieldIndex(the_col);
                } catch (...) {}
                if(col_num < 0)
                {
                    mvprintw(LINES-1,0,"*** Invalid column number ***");
                    clrtoeol();
                    refresh();
                    // wait until the user types something
                    key = getch();
                    onError = true;
                }
                else
                {
                    curj = col_num;
                    startj = max(curj-nj + 1, 0);
                }
                noecho();
            }
            break;
            ///////////////////////////////////////////////////////////////
            case (int)'v': case (int)'V':
            {
                echo();
                char strmsg[] = {"View dataset ('Enter' = reload last dataset): "};
                mvprintw(LINES-1,0,strmsg);
                clrtoeol();
                move(LINES-1, (int) strlen(strmsg));
                char c[200];
                getnstr(c, 200);
                if (!string(c).empty())
                    filename = string(c);
                VMat new_vm;
                bool error = false;
                try {
                    new_vm = getDataSet(filename);
                } catch(const PLearnError&) {
                    error = true;
                }
                if (error) {
                    mvprintw(LINES-1,0,"*** Invalid dataset ***");
                    clrtoeol();
                    refresh();
                    // Wait until the user types something.
                    key = getch();
                    onError = true;
                } else {
                    // Display the new dataset.
                    // First close the current display.
                    mvprintw(LINES-1,0,"");
                    clrtoeol();
                    refresh();
                    endwin();
                    // And launch the new one.
                    viewVMat(new_vm, filename);
                    if(string(c).empty())
                        //if we reload, we should forget the last one
                        key='q';
                }
            }
            break;
            ///////////////////////////////////////////////////////////////
            case (int)'i': case (int)'I':
            {
                echo();
                char strmsg[] = {"Insert before column ('Enter' = current, '-1' = insert at the end): "};
                mvprintw(LINES-1,0,strmsg);
                clrtoeol();
                move(LINES-1, (int)strlen(strmsg));
                char l[10];
                getnstr(l, 10);
                int ins_col = curj;
                if (l[0] != '\0') {
                    if (!pl_isnumber(l) || toint(l) < -1 || toint(l)>=vm_showed->width()) {
                        mvprintw(LINES-1,0,"*** Invalid column number ***");
                        clrtoeol();
                        refresh();
                        // wait until the user types something
                        key = getch();
                        onError = true;
                        noecho();
                        break;
                    } else {
                        ins_col = toint(l);
                    }
                }
                if (ins_col == -1)
                    ins_col = vm_showed->width();
                char strmsg2[] = {"Name of the column to insert ('Enter' = column number): "};
                mvprintw(LINES-1,0,strmsg2);
                clrtoeol();
                move(LINES-1, (int)strlen(strmsg2));
                char l2[100];
                getnstr(l2, 100);
                string ins_name = tostring(ins_col);
                if (l2[0] != '\0') {
                    ins_name = l2;
                }
                char strmsg3[] = {"Default value ('Enter' = 0): "};
                mvprintw(LINES-1,0,strmsg3);
                clrtoeol();
                move(LINES-1, (int)strlen(strmsg3));
                char l3[100];
                getnstr(l3, 100);
                string default_val = tostring(0);
                if (l3[0] != '\0') {
                    default_val = l3;
                }
                TVec<VMat> vmats;
                if (ins_col > 0)
                    vmats.append(new SubVMatrix(vm_showed, 0, 0,
                                                vm_showed->length(), ins_col));
                Mat col_mat(vm_showed->length(), 1);
                VMat col_vmat(col_mat);
                col_vmat->declareFieldNames(TVec<string>(1, ins_name));
                real val;
                if (pl_isnumber(default_val, &val))
                    col_mat.fill(val);
                else {
                    col_mat.fill(-1000);
                    col_vmat->addStringMapping(0, default_val, -1000);
                }
                vmats.append(col_vmat);
                if (ins_col < vm_showed->width())
                    vmats.append(new SubVMatrix(vm_showed, 0, ins_col,
                                                vm_showed->length(),
                                                vm_showed->width() - ins_col));
                vm_showed = new ConcatColumnsVMatrix(vmats);
                mvprintw(LINES-1,0,"*** Inserted column '%s' at position %d with default value %s ***",
                         ins_name.c_str(), ins_col, default_val.c_str());
                clrtoeol();
                refresh();
                // Wait for a key to be pressed.
                key = getch();
                noecho();
            }
            break;
            ///////////////////////////////////////////////////////////////
            case (int)'e': case (int)'E':
            {
                echo();
                char strmsg[100];
                sprintf(strmsg, "Enter column(s) or range (ex: 7;1-20;7,8,12) to export (enter=%d): ", curj);
                mvprintw(LINES-1,0,strmsg);
                clrtoeol();

                move(LINES-1, (int)strlen(strmsg));
                char strRange[50];
                getnstr(strRange, 50);

                Vec indexs;
                char strReason[100] = {"\0"};
                bool invalidInput = getList(strRange, curj, vm_showed, indexs,
                                            strReason);

                if (invalidInput)
                {
                    mvprintw(LINES-1,0,"*** Invalid input: %s ***", strReason);
                    clrtoeol();
                    refresh();
                    // wait until the user types something
                    key = getch();
                    onError = true;
                }
                else
                {

                    char filemsg[] = {"Enter file name (enter=outCol.txt): "};
                    mvprintw(LINES-1,0,filemsg);
                    clrtoeol();

                    move(LINES-1, (int)strlen(filemsg));
                    char fname[200];
                    getnstr(fname, 200);

                    if (fname[0] == '\0')
                        strcpy(fname, "outCol.txt");
                    string fname_str = fname;

                    mvprintw(LINES-1,0,"Writing file '%s'...", fname);
                    clrtoeol();
                    refresh();

                    // Save the selected columns to the desired file, keeping
                    // the string values if 'view_strings' is currently true
                    // (can be toggled with 's'/'S' keys).
                    vm_showed.columns(indexs)->saveAMAT(fname_str, false,
                                                        false, view_strings);

                    mvprintw(LINES-1,0,"*** Output written on: %s ***", fname);
                    clrtoeol();
                    refresh();
                    // wait until the user types something
                    key = getch();

                }

                noecho();
            }
            break;
            ///////////////////////////////////////////////////////////////
            case (int)'r': case (int)'R':
            {
                echo();
                char strmsg[100];
                sprintf(strmsg, "Enter column(s) or range (ex: 7;1-20;7,8,12) to view (enter=%d): ", curj);
                mvprintw(LINES-1,0,strmsg);
                clrtoeol();

                move(LINES-1, (int)strlen(strmsg));
                char c[50];
                getnstr(c, 50);

                Vec indexs;
                char strReason[100] = {"\0"};
                bool invalidInput = getList(c, curj, vm_showed, indexs,
                                            strReason);

                if (invalidInput)
                {
                    mvprintw(LINES-1,0,"*** Invalid input: %s ***", strReason);
                    clrtoeol();
                    refresh();
                    // wait until the user types something
                    key = getch();
                    onError = true;
                }
                else
                {
                    vm_showed = vm_showed.columns(indexs);
                    if (curj>=vm_showed.width())
                    {
                        curj = vm_showed.width()-1;
                        startj = max(curj-nj + 1, 0);
                    }
                }

                noecho();
            }
            break;
            ///////////////////////////////////////////////////////////////
            case (int)'x': case (int)'X':
                // Hide the currently selected row.
            {
                //echo();
                Vec index(vm_showed.width() - 1);
                for (int i = 0; i < curj; i++) {
                    index[i] = i;
                }
                for (int i = curj + 1; i < vm_showed.width(); i++) {
                    index[i - 1] = i;
                }
                vm_showed = vm_showed.columns(index);
                if (curj>=vm_showed.width()) {
                    curj = vm_showed.width()-1;
                    startj = max(curj-nj + 1, 0);
                }

                //          noecho();
            }
            break;
            ///////////////////////////////////////////////////////////////
            case (int)'<': case (int)'>':
                // Sort by increasing or decreasing order.
            {
                PP<SortRowsVMatrix> new_vm;
                if(vm_showed->classname()!="SortRowsVMatrix" 
                   || ((PP<SortRowsVMatrix>)vm_showed)->sort_columns[0]!=curj){
                //if it is a SortRowsVMatrix and we sort on a new column
                // we can't reuse the last SortRowsVMatrix as it don't suport 
                // different order on each row.
                    new_vm = new SortRowsVMatrix();
                    new_vm->source = vm_showed;
                    vm_showed = get_pointer(new_vm);
                } else 
                    //in the case where we sort multiple time on the same column
                    //we reuse the last VMatrix.
                    new_vm= (PP<SortRowsVMatrix>)vm_showed;
                new_vm->sort_columns = TVec<int>(1, curj);
                if (key == (int)'>')
                    new_vm->increasing_order = false;
                else                    
                    new_vm->increasing_order = true;
                new_vm->build();
            }
            break;
            ///////////////////////////////////////////////////////////////
            case (int)'a': case (int)'A':
                vm_showed = vm;
                break;
                ///////////////////////////////////////////////////////////////
            case (int)'n': case (int)'N':
                if ( namewidth != namewidth_ )
                    namewidth = namewidth_;
                else
                {
                    int def = (namewidth < 80) ? namewidth : 80;

                    echo();
                    char strmsg[100];
                    sprintf(strmsg, "Enter namewidth to use (between 10 and %d -- enter=%d): ",
                            namewidth, def);
                    mvprintw(LINES-1,0,strmsg);
                    clrtoeol();

                    move(LINES-1, (int)strlen(strmsg));
                    char l[10];
                    getnstr(l, 10);
                    if(l[0] == '\0')
                    {
                        namewidth = def;
                    }
                    else if( !pl_isnumber(l) || toint(l) < 0 || toint(l)>namewidth )
                    {
                        mvprintw(LINES-1,0,"*** Invalid line number ***");
                        clrtoeol();
                        refresh();
                        // wait until the user types something
                        key = getch();
                        onError = true;
                    }
                    else
                    {
                        namewidth = toint(l);
                    }
                    noecho();
                }
                break;
                //////////////////////////////////////////////////////////////
            case (int)'s':
                if (indent_strings_left)
                    // Toggle display.
                    view_strings = !view_strings;
                else {
                    // Do not remove display if we only asked to change
                    // indentation.
                    indent_strings_left = true;
                    if (!view_strings)
                        view_strings = true;
                }
                break;
            case (int)'S':
                // Same as above, except we indent to the right.
                if (!indent_strings_left)
                    view_strings = !view_strings;
                else {
                    indent_strings_left = false;
                    if (!view_strings)
                        view_strings = true;
                }
                break;
                //////////////////////////////////////////////////////////////
            case (int)'h': case (int)'H':
                erase();

                vStartHelp = 2;

                mvprintw(0,COLS/2-6,"*** HELP ***");

                mvprintw(vStartHelp++,10,"KEYS:");
                mvprintw(vStartHelp++,10," - up: move up one line");
                mvprintw(vStartHelp++,10," - down: move down one line");
                mvprintw(vStartHelp++,10," - right: move right one column");
                mvprintw(vStartHelp++,10," - left: move left one column");
                mvprintw(vStartHelp++,10," - page up: move up one screen");
                mvprintw(vStartHelp++,10," - page down: move down one screen");
                mvprintw(vStartHelp++,10," - home: move to the first column");
                mvprintw(vStartHelp++,10," - end: move to the last column");
                mvprintw(vStartHelp++,10," - 'r' or 'R': show only a range or a set of columns");
                mvprintw(vStartHelp++,10," - 'x' or 'X': hide the currently selected column");
                mvprintw(vStartHelp++,10," - 'a' or 'A': show the original VMat");
                mvprintw(vStartHelp++,10," - 'f' or 'F': toggle the display of the filename");
                mvprintw(vStartHelp++,10," - 'i' or 'I': insert a new column with default value");
                mvprintw(vStartHelp++,10," - 'l' or 'L': prompt for a line number and go to that line");
                mvprintw(vStartHelp++,10," - 'c' or 'C': prompt for a column number and go to that column");
                mvprintw(vStartHelp++,10," - 's' or 'S': toggle display string fields as strings or numbers ('S' = right indentation)");
                mvprintw(vStartHelp++,10," - 't' or 'T': toggle transposed display mode");
                mvprintw(vStartHelp++,10," - 'n' or 'N': toggle truncated field name display mode (under transposed display mode)");
                mvprintw(vStartHelp++,10," - 'e' or 'E': export a range or a set of columns to file");
                mvprintw(vStartHelp++,10," - 'v' or 'V': prompt for another dataset to view");
                mvprintw(vStartHelp++,10," - '.'       : toggle displaying of ... for values that do not change (exact match)");
                mvprintw(vStartHelp++,10," - ','       : toggle displaying of ... for values that do not change (approximate match)");
                mvprintw(vStartHelp++,10," - '/'       : search for a value of the current field");
                mvprintw(vStartHelp++,10," - '<' or '>': sort column by increasing or decreasing order");
                mvprintw(vStartHelp++,10," - 'h' or 'H': display this screen");
                mvprintw(vStartHelp++,10," - 'q' or 'Q': quit program");
                mvprintw(vStartHelp++,COLS/2-13,"(press any key to continue)");

                refresh();
                getch();

                break;

            case (int)'f': case (int)'F':
                display_filename = !display_filename;

            case (int)'q': case (int)'Q':
                break;

                //////////////////////////////////////////////////////////////
            default:
                mvprintw(LINES-1,0,"*** Invalid command (type 'h' for help) ***");
                // clear the rest of the line
                clrtoeol();

                refresh();

                // wait until the user types something
                key = getch();
                onError = true;

                //sleep(1);
                break;
            }
        }
    } // end try
    catch(const PLearnError& e)
    {
        endwin();
        throw(e);
    }

    // make sure it is clean
    mvprintw(LINES-1,0,"");
    clrtoeol();
    refresh();

    endwin();
}


BEGIN_DECLARE_REMOTE_FUNCTIONS

    declareFunction("viewVMat", &viewVMat,
                    (BodyDoc("Displays a VMat's contents using curses.\n"),
                     ArgDoc("vm",
                            "the VMat to display"),
                     ArgDoc("filename",
                            "optional filename of the dataset, that may be used to reload it (\"\" works just fine)")));

END_DECLARE_REMOTE_FUNCTIONS


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
