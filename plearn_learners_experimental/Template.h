#ifndef Template_INC
#define Template_INC

#include <plearn/math/TMat_maths.h>
#include <string>
#include "Molecule.h"

namespace PLearn {

class Template;
typedef PP< Template > MoleculeTemplate;

class Template : public Molecule{

private:
		typedef Molecule inherited;


    public:
        //the properties of the template
        PLearn::Mat dev;
        
        Template(); 
        /*
        Template(const Template &  t) {
                    
            chem.resize(t.chem.length(), t.chem.width() ) ; 
            chem << t.chem ; 

            geom.resize(t.geom.length(), t.geom.width() ) ; 
            geom << t.geom ; 
            
            dev.resize(t.dev.length(), t.dev.width() ) ; 
            dev << t.dev ; 
        }
        */
		// virtual ~Template() {}

private: 
  //! This does the actual building. 
  void build_();

protected: 
  //! Declares this class' options.
  static void declareOptions(OptionList& ol);

public:
  // Declares other standard object methods.
  PLEARN_DECLARE_OBJECT(Template);

  // simply calls inherited::build() then build_() 
  virtual void build();

  //! Transforms a shallow copy into a deep copy
  virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);



};
// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(Template);

}
#endif
