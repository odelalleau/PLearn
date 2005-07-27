from datetime import *

def cyymmdd_to_date(x):
    cyymmdd  = int(x)
    cyymm,dd = divmod(cyymmdd,100)
    cyy,mm   = divmod(cyymm,100)
    yyyy = 1900+cyy

    return date(yyyy,mm,dd)
  
def date_to_cyymmdd(d):
    return (d.year-1900)*10000 + d.month*100 +d.day

def cyymmdd_to_ordinal(x):
    if isNaN(x):
      return x
    return cyymmdd_to_date(x).toordinal()

def ordinal_to_cyymmdd(ordinal):
    return date_to_cyymmdd(date.fromordinal(int(ordinal)))

