def transformfunc(fname, text):
    if fname.endswith('.h'):
        text = re.sub('using namespace std;\n', '', text)
    return text
