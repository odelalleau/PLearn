
#include "icpalign.h"

int main () {

    string name1 = "89.vrml" ; 
    string name2 = "92.vrml" ; 
    
    PMolecule m1 , m2 ;  
    m1 = Molecule::readMolecule(string("89")) ; 
    m2 = Molecule::readMolecule(string("92")) ; 
    
    Mat wm ; 
    ::align(name1,m1->chem ,name2,m2->chem,wm)  ; 
    
    for(int i=0 ; i<wm.length() ; ++i) { 
        for(int j=0 ; j<wm.width() ; ++j) { 
            if (wm[i][j] > 0.5)
                cout << i+1 << " " << j+1 << endl ; 
        }

    }

}


