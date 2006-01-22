"""Contains the tools to create dynamic tutorial."""
import os
from plearn.utilities import toolkit

def getIndentedScripts( pyplearn_path, indent ):
    # Reading file from script and indenting
    indented_pyscript = ""
    pyplearn_script = open( pyplearn_path, 'r' )

    first = True
    for line in pyplearn_script:
        if first:
            indented_pyscript += line
            first = False
        else:            
            indented_pyscript += indent + line
    pyplearn_script.close()

    # Parsing the script using the pyplearn_driver
    indented_plscript = ""
    plearn_script   = toolkit.command_output( "pyplearn_driver.py %s" %
                                              pyplearn_path )
    first = True
    for line in plearn_script:
        if first:
            indented_plscript += line
            first = False
        else:
            indented_plscript += indent + line

    return indented_pyscript, indented_plscript


def dedent( s, itoken = ' '*4 ):    
    lines = s.split('\n')

    dedented = []
    itklen   = len( itoken )
    for line in lines:
        if line.startswith( itoken ):
            dedented.append( line[itklen:] )
        else:
            dedented.append( line )
    return '\n'.join( dedented )

class Section:    
    section_counter = 1
    
    def __init__( self, title, content, subsections=[], no=None ):        
        if no is None:
            self.no = Section.section_counter
            Section.section_counter += 1
        else:
            self.no = no
        
        self.title       = title
        self.content     = content
        self.subsections = subsections

    def __str__( self ):
        header = self.markHeader( "Section %s: %s" % (self.no, self.title) )

        subcontent = ''
        for subsection in self.subsections:
            subcontent += '\n\n'+str(subsection) 

        return "%s\n%s%s" % (header, self.content, subcontent)

    def markHeader( self, header ):
        marker = "="*len(header)
        return '%s\n%s' % ( header, marker )
        

class SubSection( Section ):
    owner              = None
    subsection_counter = None
    
    def __init__( self, title, content ):
        if SubSection.owner != Section.section_counter:
            SubSection.subsection_counter = 1
            SubSection.owner = Section.section_counter
            
        no = "%d.%d" % ( Section.section_counter,
                         SubSection.subsection_counter )
        SubSection.subsection_counter += 1

        Section.__init__( self, title, content, no = no )

    def markHeader( self, header ):
        indent = ' '*4
        marker = "-"*len(header)
        return '%s%s\n%s%s' % ( indent, header, indent, marker )
    

class Tutorial:
    """Base class for tutorials evolving with the source code.

    Any subclass of C{Tutorial} is a considered to be a dynamic
    tutorial. The docstring of the derived class is considered to be the
    introduction of the tutorial: the one-line comment is used as
    title. The subclass is expected to defines a classmethod C{sections}
    which should return a list of the L{Section} objects to be added to the
    tutorial
    """
    wstr = toolkit.WString()

    def sections( cls ):
        raise NotImplementedError
    sections = classmethod( sections )

    def build( cls, tutorial_file ):
        """Builds the tutorial represented by this class.

        Read the C{Tutorial} class' documentation for details.
        """
        assert os.path.exists( tutorial_file )
        
        print >>cls.wstr, dedent( cls.__doc__ )
        for section in cls.sections():
            print >>cls.wstr, section
        tutorial_str = '"""%s"""' % str(cls.wstr).replace( '"""', '\\"\\"\\"' )

        #
        #  Edit the tutorial module's docstring
        #
        tfile = open( tutorial_file, 'r' ) 
        fstr  = tfile.read()
        tfile.close()

        start = fstr.find('"""')
        if start == 0:
            # The module already has a docstring
            stop = fstr.find('"""', 3)
            stop = fstr.find('\n', stop) 
            fstr = fstr.replace( fstr[start:stop], tutorial_str )
        else:
            fstr = '\n'.join([ tutorial_str, fstr ])

        # Rewriting the module
        tfile = open( tutorial_file, 'w' ) 
        tfile.write( fstr )
        tfile.close()
        
    build = classmethod( build )

