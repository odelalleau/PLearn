def transformfunc(fname, text):
    new_lines = []
    for line in text.split('\n'):
        if not (line.startswith('#') or line.startswith('//#')):
            for word in ['ostream', 'istream', 'string', 'iostream'
                         'streambuf', 'istringstream', 'ostringstream',
                         'map<', 'list<', 'pair<', 'multimap<', 'set<',
                         'ifstream', 'ofstream', 'fstream', 'ios_base',
                         'istrstream', 'ostrstream', 'ios',
                         'cout', 'cin', 'cerr', 'clog',
                         'random_access_iterator_tag',
                         'greater<', 'vector<']:
                line = re.sub(r'\b(?<!std::)' + word + r'\b',
                              'std::' + word, line)
                line = re.sub(r'\b(?<!std::)' + word + r'(?= )',
                              'std::' + word, line)
            for word in ['hash_map<', 'hash_multimap<' ]:
                line = re.sub(r'\b(?<!__gnu_cxx::)' + word + r'\b',
                              '__gnu_cxx::' + word, line)                
        new_lines.append(line)
    return '\n'.join(new_lines)
    
