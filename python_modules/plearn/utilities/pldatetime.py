from datetime                  import *
from fpconst                   import isNaN
from plearn.utilities.Bindings import Bindings

#
#  Module functions
#
def cyymmdd_to_date(x):
    cyymmdd  = int(x)
    cyymm,dd = divmod(cyymmdd,100)
    cyy,mm   = divmod(cyymm,100)
    yyyy = 1900+cyy
    return date(yyyy,mm,dd)

def yyyymmdd_to_date(x):    
    yyyymmdd  = int(x)
    yyyymm,dd = divmod(yyyymmdd,100)
    yyyy,mm   = divmod(yyyymm,100)

    return date(yyyy,mm,dd)

def pljulian_to_date(x):
    """Convert a PLearn Julian Number to a Python date.
    """
    assert not isNaN(x)
    return date.fromordinal(int(x - 1721425))
  
def date_to_cyymmdd(d):
    return (d.year-1900)*10000 + d.month*100 +d.day

def date_to_yyyymmdd(d):
    return d.year*10000 + d.month*100 +d.day

def cyymmdd_to_ordinal(x):
    if isNaN(x):
      return x
    return cyymmdd_to_date(x).toordinal()

def ordinal_to_cyymmdd(ordinal):
    return date_to_cyymmdd(date.fromordinal(int(ordinal)))

def convertPLDateTime(cyymmdd, hhmnss):
    cyymmdd  = int(cyymmdd)
    cyymm,dd = divmod(cyymmdd,100)
    cyy,mm   = divmod(cyymm,100)
    yyyy = 1900+cyy

    hhmnss = int(hhmnss)
    hhmn,ss = divmod(hhmnss, 100)
    hh,mn = divmod(hhmn, 100)

    return datetime(yyyy, mm, dd, hh, mn, ss)

__tic = []
def tic():
    import time
    global __tic
    __tic.append( datetime(*time.localtime()[:6]) )

def toc():
    import time
    global __tic
    now = datetime(*time.localtime()[:6])
    if len(__tic) == 0:
        import sys
        print >>sys.stderr, "Call to toc() corresponding to no tic()..."
        return -1
    return now - __tic.pop(-1)

#
#  Module classes
#
class YearIndices:
    def __init__( self, start_index, start_date ):        
        self.start_date = date( start_date.year, 01, 01 )
        self.last_date  = date( start_date.year, 12, 31 )
        
        self.year       = start_date.year
        self.months     = Bindings()
        self.indices    = []

        self.add( start_index, start_date )

    def __getitem__( self, month ):
        return self.months[ month ]

    def __str__( self ):
        return ( "Year %s: Start at %d (%s observations)\n  " % (self.year, self.indices[0], len(self.indices))
                 + "\n  ".join([ str(m) for m in self.months.itervalues() ])
                 )

    def __iter__( self ):
        return self.months.iteritems()

    def add( self, index, date ):
        self.indices.append( index )

        try:
            self.months[ date.month ].add( index, date )
        except ( KeyError, ValueError ), err:
            self.months[ date.month ] = MonthIndices( index, date )
    

class MonthIndices:
    def __init__( self, start_index, start_date ):
        self.start_date = date( start_date.year, start_date.month, 01 )
        if start_date.month == 12:
            self.last_date = date( start_date.year, 12, 31 )
        else:
            self.last_date = date( start_date.year, start_date.month+1, 01 ) - timedelta(1)
        
        self.month      = start_date.month
        self.months     = Bindings()
        self.indices    = []

        self.add( start_index, date )

    def __str__( self ):
        return "Month %s: Start at %d (%s observations)" % (self.month, self.indices[0], len(self.indices))

    def add( self, index, date ):
        self.indices.append( index )

class IndexedDates:
    def __init__( self, dates ):
        self.dates = dates
        self.years = Bindings()
        for index, date in enumerate(dates):
            try:
                self.years[ date.year ].add( index, date )
            except ( KeyError, ValueError ), err:
                self.years[ date.year ] = YearIndices( index, date )

    def __getitem__( self, datelike ):
        if isinstance( datelike, date ):
            return self.dates.index( date )
        
        elif len(datelike) == 1:
            return self.years[ datelike ]

        elif len(datelike) == 2:
            return self.years[ datelike[0] ][ datelike[1] ]

        else:
            raise ValueError(datelike) 

    def __iter__( self ):
        return self.years.iteritems()

    def __str__( self ):
        return "\n\n".join([ str(y) for y in self.years.itervalues() ])

