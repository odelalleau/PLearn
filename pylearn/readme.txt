Il faut compiler avec pymake -so 
Dans pymake_config_model, il y a un trigger qui detecte automatiquement boost/python.hpp et qui link avec ce qu'il faut. (config d'ApsTAT pur l'instant)

Mais pymake -so pylearn va creer qqch du genre OBJS/linux-i386__g++_dbg_double_throwerrors_blas/libpylearn.so
Or python s'attend a ce que le fichier .so ait le meme nom que dans BOOST_PYTHON_MODULE(...) c.a.d. pylearn

J'ai donc fait un lien symbolique de pylearn.so -> OBJS/linux-i386__g++_dbg_double_throwerrors_blas/libpylearn.so

Il suffit alors de faire 'import pylearn' dans python.

-- Pascal

