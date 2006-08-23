#! /usr/bin/env python2.4

import sys,time,random,md5,glob,string
from configobj import ConfigObj


def get_new_sid(tag):
    """Build a new Session ID"""
    t1 = time.time()
    time.sleep( random.random() )
    t2 = time.time()
    base = md5.new( tag + str(t1 +t2) )
    sid = tag + '_' + base.hexdigest()
    return sid

def file_exists(filename):
    return len(glob.glob(filename)) > 0

def set_config_value(file, keyword, value):
    config = ConfigObj(file) 
    config[keyword] = value
    config.write()

def get_config_value(file, keyword):
    config = ConfigObj(file)
    try: 
        return config[keyword]
    except KeyError:
        return -1

def set_current_date(file, keyword,time_format):
    config = ConfigObj(file) 
    config[keyword] = time.strftime(time_format, time.localtime(time.time()))
    config.write()
    
def truncate(s, length):
    if len(s) < length:
        return s
    else:
        return s[:length] + etc
    
def string_replace(s,c,ch=''):
    """Remove any occurrences of characters in c, from string s
       s - string to be filtered, c - characters to filter"""
    for a in c:
        s = s.replace(a,ch)
        
    return s

def create_eval_command(function_name , args):
    return function_name +"('" +  string.join(args,"','") + "')"

if __name__ == '__main__':
    #    cmd = sys.argv[1] +"('" +  string.join(sys.argv[2:],"','") + "')"
    cmd = create_eval_command(sys.argv[1], sys.argv[2:])
    eval(cmd)
