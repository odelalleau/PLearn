
#include "icpalign.h"

int main (int argc, char ** argv) {

    if (argc < 3) {cout<< "error , provide model and scene name" ; return 0 ; }
    
    string name1 = string(argv[1]) + ".vrml" ; 
    string name2 = string(argv[2]) + ".vrml" ;
    
    PMolecule m1 , m2 ;  
    m1 = Molecule::readMolecule(string(argv[1])) ; 
    m2 = Molecule::readMolecule(string(argv[2])) ; 
    
    Mat wm ; 
    ::align(name1,m1->chem ,name2,m2->chem,wm)  ; 
    
    for(int i=0 ; i<wm.length() ; ++i) { 
        for(int j=0 ; j<wm.width() ; ++j) { 
            if (wm[i][j] > 0.5)
                cout << i+1 << " " << j+1 << endl ; 
        }

    }

}


