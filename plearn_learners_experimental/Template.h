#ifndef Template_INC
#define Template_INC

#include <plearn/math/TMat_maths.h>
#include <string>
#include "Molecule.h"


class Template : public Molecule{
    public:
        //the properties of the template
        PLearn::Mat dev;
        
        Template(); 
        Template(const Template &  t) {
                    
            chem.resize(t.chem.length(), t.chem.width() ) ; 
            chem << t.chem ; 

            geom.resize(t.geom.length(), t.geom.width() ) ; 
            geom << t.geom ; 
            
            dev.resize(t.dev.length(), t.dev.width() ) ; 
            dev << t.dev ; 
        }

        Template & operator= (const Template & t) {
            chem.resize(t.chem.length(), t.chem.width() ) ; 
            chem << t.chem ; 

            geom.resize(t.geom.length(), t.geom.width() ) ; 
            geom << t.geom ; 

            dev.resize(t.dev.length(), t.dev.width() ) ; 
            dev << t.dev ; 
            
            return *this ; 
        }

};
#endif
