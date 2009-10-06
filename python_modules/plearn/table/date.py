"""
Module plearn.table.date

Copyright (C) 2005 ApSTAT Technologies Inc.

This file was contributed by ApSTAT Technologies to the
PLearn library under the following BSD-style license: 

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

"""

# Author: Pascal Vincent

import datetime, string
from re import match

monthnum = {
'JAN' : 1,
'FEB' : 2,
'MAR' : 3,
'APR' : 4,
'MAY' : 5,
'JUN' : 6,
'JUL' : 7,
'AUG' : 8,
'SEP' : 9,
'OCT' : 10,
'NOV' : 11,
'DEC' : 12 }

def CYYMMDD_to_YYYYMMDD(cyymmdd):
    return 19000000+int(cyymmdd)

def YYYYMMDD_to_CYYMMDD(yyyymmdd):
    return int(yyyymmdd)-19000000

def date_to_CYYMMDD(date):
    """Takes a python datetime.date and returns an int in CYYMMDD format"""
    if date.year<1900:
        raise ValueError('In date_to_CYYMMDD CYYMMDD format is only valid for years 1900 and above, not'+str(date))
    return (date.year-1900)*10000 + date.month*100 + date.day

def date_to_YYYYMMDD(date):
    """Takes a python datetime.date and returns an int in YYYYMMDD format"""
    return date.year*10000 + date.month*100 + date.day

def CYYMMDD_to_date(cyymmdd):
    cyymm,dd = divmod(cyymmdd,100)
    cyy,mm = divmod(cyymm,100)
    try: d = datetime.date(1900+cyy,mm,dd)
    except ValueError:
        raise ValueError('Invalid CYYMMDD date: ' + str(cyymmdd))
    return d

daysinmonth = [31,28,31,30,31,30,31,31,30,31,30,31]
def float_sum(l):
    s = 0
    for x in l:
        s += float(x or '0')
    return s

def CYYMMDD_to_ymd(cyymmdd):
    y = int(cyymmdd/10000)
    mmdd = cyymmdd - 10000*y
    m = int(mmdd/100)
    d = mmdd - 100*m
    return (y,m,d)
    
def toyears(date):
    sdate = str(date)
#    print ':',sdate,':'
    # Assumed format:  DDMMMYYYY
    if match(r'\d{2}[a-zA-Z]{3}\d{4}',sdate): 
        try:
            d = float(sdate[0:2])
            m = monthnum[sdate[2:5]]
            y = float(sdate[5:])
        except:
            raise ValueError('1:Can\'t convert ' + str(date) + ' in toyears()')
        
    # Assumed format:  YYYYMMDD
    elif match(r'\d{8}',sdate):  
        try:
            d = date%100
            date = (date - d)/100
            m = date%100
            y = (date - m)/100
        except:
            raise ValueError('2:Can\'t convert ' + str(date) + ' in toyears()')

    # Assumed format:  YYMMDD        
    elif match(r'\d{6}',sdate):
        try:
            d = date%100
            date = (date - d)/100
            m = date%100
            y = (date - m)/100 + 1900
            if y < 4:
                y = y + 100
        except:
            raise ValueError('3:Can\'t convert ' + str(date) + ' in toyears()')        

    # Assumed format:  YYYY        
    elif match(r'\d{4}',sdate):
	try:
	    y = float(date)
	    m = 1
	    d = 1
	except:
            raise ValueError('4:Can\'t convert ' + str(date) + ' in toyears()')
	    
    else:
        raise ValueError('0:Can\'t convert ' + str(date) + ' in toyears()')
    if y%4==0:
        if m > 2:
            return y + (float_sum(daysinmonth[:m-1])+1+d-1)/366.
        else:
            return y + (float_sum(daysinmonth[:m-1])+d-1)/366.
    return y + (float_sum(daysinmonth[:m-1])+d-1)/365.

def YYYYMMDD_to_date(yyyymmdd):
    yyyymm,dd = divmod(yyyymmdd,100)
    yyyy,mm = divmod(yyyymm,100)
    return datetime.date(yyyy,mm,dd)
    
def dateint_to_date(dateint):
    """Takes a dateint in YYYYMMDD or CYYMMDD format and returns a python datetime.date"""
    if dateint>=10000000: # YYYYMMDD format (we suppose we won't see date before year 1000 !)
        return YYYYMMDD_to_date(dateint)
    else: # CYYMMDD format
        return CYYMMDD_to_date(dateint)

def datestring_to_date(datestring):
    """Takes a datestring in various formats and returns a python datetime.date
    Currently recognized formats are:
    YYYYMMDD
    CYYMMDD
    YYYY-MM-DD
    YYYY/MM/DD
    27JAN2003
    """
    date = datestring.strip()
    dateint = 0
    try: dateint = int(datestring)
    except: pass
    if dateint:
        return dateint_to_date(dateint)

    # Format "2003/01/27" or "2003-01-27"
    if len(date) == 10 and date[4] in '/-' and date[7] in '/-':
        year = int(date[0:4])
        month = int(date[5:7])
        day = int(date[8:10])
        return datetime.date(year,month,day)
        
    # Format "27JAN2003"
    if len(date)==9 and date[2] in string.uppercase and date[3] in string.uppercase and date[4] in string.uppercase:
        year = int(date[5:9])
        day = int(date[0:2])
        month = monthnum[date[2:5]]
        return datetime.date(year,month,day)
        
    raise ValueError("Invalid datestring format: "+datestring)

def datestring_to_CYYMMDD(datestring):
    return date_to_CYYMMDD(datestring_to_date(datestring))

def datestring_to_YYYYMMDD(datestring):
    return date_to_YYYYMMDD(datestring_to_date(datestring))

def todate(date):
    """Accepts both string date formats and int date formats, and returns a python datetime.date"""
    if isinstance(date,datetime.date):
        return date
    elif type(date)==str:
        return datestring_to_date(date)
    elif type(date)==int:
        return dateint_to_date(date)
        
def daydiff(date1, date2):
    return (dateint_to_date(date1) - dateint_to_date(date2)).days
