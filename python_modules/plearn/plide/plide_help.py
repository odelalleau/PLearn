#  plide_help.py
#  Copyright (C) 2006 by Nicolas Chapados
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are met:
#
#   1. Redistributions of source code must retain the above copyright
#      notice, this list of conditions and the following disclaimer.
#
#   2. Redistributions in binary form must reproduce the above copyright
#      notice, this list of conditions and the following disclaimer in the
#      documentation and/or other materials provided with the distribution.
#
#   3. The name of the authors may not be used to endorse or promote
#      products derived from this software without specific prior written
#      permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
#  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
#  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
#  NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
#  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
#  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
#  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
#  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#  This file is part of the PLearn library. For more information on the PLearn
#  library, go to the PLearn Web site at www.plearn.org

import sys
import gtk
import gobject
import gtkhtml2

from plide_utils import markup_escape_text


_html_page_not_found = """
<html>
<title>Page not accessible</page>
<body>
<h1>Page not accessible</h1>
The page at location "%s" cannot be displayed.<p>
Only local PLearn help pages can be displayed by the Plide help system.
</body>
</html>
"""

class PlideHelp( object ):
    """Simple manager for the Plide Help system.
    """
    def __init__( self, plide_main ):
        self.plide_main = plide_main

        ## List of back and forward pages
        self.back_pages = []
        self.fwd_pages  = []
        self.cur_page   = None

        ## Connect back and forward buttons to event handlers
        plide_main.w_help_back.connect("clicked", self.go_back)
        plide_main.w_help_forward.connect("clicked", self.go_forward)

        ## Set up HTML document
        self.doc = gtkhtml2.Document()
        self.doc.connect("request-url",  self.request_url)
        self.doc.connect("link-clicked", self.link_clicked)

        ## Set up HTML viewer widget
        self.view = gtkhtml2.View()
        self.view.set_property("can-focus",False)
        self.view.set_document(self.doc)
        self.view.set_magnification(1.0)
        self.view.show_all()

        plide_main.w_help_scrolledwindow.add(self.view)
        
    def define_injected( inj ):
        """Simply transmits the 'injected' pseudo-module to this module.
        """
        global injected
        injected = inj
    define_injected = staticmethod(define_injected)

    def request_url( self, doc, uri, stream ):
        """Callback that's invoked when HTML widget needs to load a URI.
        """
        pass

    def link_clicked( self, doc, uri ):
        """Callback that's invoked when HTML widget reports that a link has
        been clicked by the user.
        """       
        self.display_page(uri)

    def display_page( self, uri, doc = None ):
        """Take the current page to the back list, clear the forward list,
        and update the title label.
        """
        if self.cur_page is not None:
            self.back_pages.append(self.cur_page)
        self.cur_page  = uri
        self.fwd_pages = []
        
        title = self.update_doc_from_uri(uri)
        self.update_title(title)

    def go_back( self, widget ):
        if self.back_pages:
            if self.cur_page:
                self.fwd_pages.append(self.cur_page)
            self.cur_page = self.back_pages.pop()
            title = self.update_doc_from_uri(self.cur_page)
            self.update_title(title)

    def go_forward( self, widget ):
        if self.fwd_pages:
            if self.cur_page:
                self.back_pages.append(self.cur_page)
            self.cur_page = self.fwd_pages.pop()
            title = self.update_doc_from_uri(self.cur_page)
            self.update_title(title)

    def update_title( self, new_title ):
        """Update the title label, and shade/unshade back-forward controls
        depending on contents of the back-forward list.
        """
        new_title = markup_escape_text(new_title)
        self.plide_main.w_help_title.set_markup("<b>%s</b>" % new_title)
        self.plide_main.w_help_back   .set_sensitive(len(self.back_pages) > 0)
        self.plide_main.w_help_forward.set_sensitive(len(self.fwd_pages ) > 0)

    def update_doc_from_uri( self, uri, doc = None ):
        """Pure dispatch of HTML into document, without managing the
        back-forward history or updates to the title.  Return the new page
        title.
        """
        if doc is None:
            doc = self.doc

        # params
        posparams= uri.find('?')
        poslev= uri.find("level=", posparams)
        if poslev != -1:
            posend= uri.find(";",poslev)
            if posend == -1:
                posend= len(uri)
            injected.setOptionLevel(int(uri[poslev+6:posend]))
        if posparams != -1:
            # get rid of params here
            uri= uri[:posparams]

        html  = ""
        title = ""
        if uri == "index.html":
            html  = injected.helpIndex()
            title = "Help Index"

        elif uri == "class_index.html":
            html  = injected.helpClasses()
            title = "Class Index"

        elif uri == "command_index.html":
            html  = injected.helpCommands()
            title = "Command Index"

        elif uri.startswith("command_") and uri.endswith(".html"):
            command = uri[8:-5]         # Drop prefix and suffix
            title   = command + " (command)"
            html = injected.helpOnCommand(command)

        elif uri.startswith("class_") and uri.endswith(".html"):
            classname = uri[6:-5]       # Drop prefix and suffix
            title     = classname + " (class)"
            html = injected.helpOnClass(classname)

        else:
            html  = _html_page_not_found % uri
            title = "Page not accessible"

        doc.open_stream("text/html")
        doc.write_stream(html)
        doc.close_stream()

        ## Set the scrollbar to top of document to preserve some consistency.
        self.plide_main.w_help_scrolledwindow.get_vadjustment().set_value(0)
        return title
