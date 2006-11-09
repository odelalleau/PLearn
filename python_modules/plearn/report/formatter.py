import sys
from plearn.pyplearn.plearn_repr import plearn_repr

DEFAULT_WRITER = sys.stdout.write

def elemFormat(e):
    if isinstance(e, str):
        return e # Not quoted...
    elif isinstance(e, float):
        return "%.2f"%e
    return plearn_repr(e)

def twikiTableLine(table_line, elem_format=None, writer=DEFAULT_WRITER):    
    format = lambda e : (elem_format and elem_format%e) or elemFormat(e)
    writer('|  ')
    for element in table_line:
        writer("%s  |  " % format(element))
    writer("\n")

def twikiTable(table, headers=[], writer=DEFAULT_WRITER):
    writer( '%TABLE{ sort="off" tableborder="0" '
            'cellpadding="2" cellspacing="1" headerbg="#000099" headercolor="#FFFFCC" '
            'databg="#C8CB8F, #DBDDB5" headerrows="1" footerrows="0" }%\n' )

    if headers:
        twikiTableLine(headers, "*%s*", writer)

    for line in table:
        twikiTableLine(line, writer=writer)

def latexTable(table, headers=[],
               align="", padding=0.5, caption="", label="",
               fontsize="", writer=DEFAULT_WRITER):
    lwriter = lambda line : writer("%s\n"%line)
    if align:
        assert len(align)==len(table[0]), "%d != %d"%(len(align), len(table[0])) 
    else:
        align = "c"*len(table[0])

    if padding:
        padded = "%s@{\\hspace{%.3fcm}}"%(align[0],padding)
        for a in align[1:-1]:
            padded += "%s@{\\hspace{%.3fcm}}"%(a,2*padding)
        if len(align) > 1:
            padded += "%s@{\\hspace{%.3fcm}}"%(align[-1],padding)
        align = padded    
        
    lwriter("\\begin{table}[!htp]")
    lwriter("\\begin{center}")
    if fontsize:
        lwriter(fontsize)
    lwriter("\\begin{tabular}{%s}"%align)
    
    if headers:
        writer("  ")
        latexTableLine(headers, writer)
        lwriter("\\hline\\hline")
    else:
        lwriter("\\hline")
 
    for line in table:
        writer("  ")
        if isinstance(line, str):
            lwriter(line) # Single string is wrote as is...
        else:
            latexTableLine(line, writer)
    lwriter("\\hline")
       
    lwriter("\\end{tabular}")
    if caption:
        lwriter("    \\tabcaption{%s}"%caption)
    if label:
        lwriter("    \\label{table:%s}"%label)
    lwriter("\\end{center}")
    lwriter("\\end{table}")                

def latexTableLine(line, writer=DEFAULT_WRITER):
    for elem in line[:-1]:
        if elem is not None: # For \multicolumn...
            writer(" %s &"%elem)
    writer(" %s \\\\ \n"%line[-1])

def strictlyUpperTriangularTable(table, headers=[], format="%s"):
    """Returns a table of strings and modified headers suitable for latex/twikiTable.

    The 'table' is assumed to be a square matrix of numbers in which the
    subdiagonal AND diagonal elements are to be neglected. It can also be a
    pair of tables, in which case the format is assumed to handles pairs.
    """
    N = len(table)
    rows = iter(table)
    columns = lambda row: iter(row)
    if isinstance(table, tuple):
        table, subtable = table
        N = len(table)
        rows = iter( zip(table,subtable) )
        columns = lambda row: iter( zip(row[0], row[1]) )        
    assert N==len(table[0])

    formatted = []
    for i, row in enumerate(rows):
        if i == N-1:
            break
        
        formatted_row = [ ]
        if headers:
            formatted_row.append(headers[i])
            
        for j, col in enumerate( columns(row) ):
            if j == 0:
                continue
            elif j <= i:
                formatted_row.append('-'),
            else:
                formatted_row.append( format%col )
        formatted.append(formatted_row)

    return formatted, ['']+headers[1:]
