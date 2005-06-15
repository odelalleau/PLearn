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

def section( title ):
    sec = '='*len(title)
    return '%s\n%s' % ( title, sec )
                
class WString:
    """Writable String.

    Has a write() method.
    """
    def __init__( self, s='' ):
        self.s = s

    def __str__( self ):
        return self.s

    def write( self, s ):
        self.s += s

class Tutorial:
    wstr = WString()
    
    def build( cls, tutorial_file ):
        assert os.path.exists( tutorial_file )
        
        print >>cls.wstr, dedent( cls.__doc__ )
        for chp, chapter in enumerate( cls.chapters() ):
            if isinstance( chapter, tuple ):
                chapter, chapter_name = chapter
            else:
                chapter_name = chapter.__name__

            if not isinstance( chapter, str ):
                chapter = ' '*4 + chapter.__doc__
                                
            print >>cls.wstr, section( "Chapter %d: %s" % (chp+1, chapter_name) )
            print >>cls.wstr, chapter

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

