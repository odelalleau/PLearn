
def transformfunc(fname, text):
    text = re.sub(r'\bparentclass\b', 'inherited', text)    

    text, commentlist, stringlist = hideCommentsAndStrings(text)

    text = transformSimpleCall(text, 'DECLARE_NAME_AND_DEEPCOPY', 1, 'PLEARN_DECLARE_OBJECT(%1)')
    text = transformSimpleCall(text, 'IMPLEMENT_NAME_AND_DEEPCOPY', 1, 'PLEARN_IMPLEMENT_OBJECT(%1, "ONE LINE DESCR", "NO HELP")')
    text = transformSimpleCall(text, 'DECLARE_ABSTRACT_NAME_AND_DEEPCOPY', 1, 'PLEARN_DECLARE_ABSTRACT_OBJECT(%1)')
    text = transformSimpleCall(text, 'IMPLEMENT_ABSTRACT_NAME_AND_DEEPCOPY', 1, 'PLEARN_IMPLEMENT_ABSTRACT_OBJECT(%1, "ONE LINE DESCR", "NO HELP")')

    text = transformSimpleCall(text, 'PLEARN_DECLARE_OBJECT_METHODS', 3, 'PLEARN_DECLARE_OBJECT(%1)')
    text = transformSimpleCall(text, 'PLEARN_IMPLEMENT_OBJECT_METHODS', 3, 'PLEARN_IMPLEMENT_OBJECT(%1, "ONE LINE DESCR", "NO HELP")')
    text = transformSimpleCall(text, 'PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS', 3, 'PLEARN_DECLARE_ABSTRACT_OBJECT(%1)')
    text = transformSimpleCall(text, 'PLEARN_IMPLEMENT_ABSTRACT_OBJECT_METHODS', 3, 'PLEARN_IMPLEMENT_ABSTRACT_OBJECT(%1, "ONE LINE DESCR", "NO HELP")')

    # Suprimer deepRead et deepWrite
    text = re.sub(r'\bvoid\s+\w+::deepRead\([^\}]+\}', '', text)
    text = re.sub(r'\bvoid\s+\w+::deepWrite\([^\}]+\}', '', text)
    text = re.sub(r'\b\bvirtual\s+void\s+deepRead\([^\;]+\;', '', text)
    text = re.sub(r'\b\bvirtual\s+void\s+deepWrite\([^\;]+\;', '', text)

    text = restoreCommentsAndStrings(text, commentlist, stringlist)
    return text



#     text = re.sub(r'#include\s+\"general\.h\"', '#include "PLearnCore/general.h"', text)
#     text, commentlist, stringlist = hideCommentsAndStrings(text)
#     text = transformDotMethodCall(text, 'accumulate', 1, '%0 += %1')
#     text = transformArrowMethodCall(text, 'accumulate', 1, '*(%0) += %1')
