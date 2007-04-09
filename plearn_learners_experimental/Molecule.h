
#ifndef Molecule_INC
#define Molecule_INC


#include <plearn/io/load_and_save.h>
#include <plearn/math/TMat_maths.h>
#include <plearn/base/Object.h>
#include "SurfaceMesh.h"
#include "geometry.h"
#include <vector>

#include <string>

namespace PLearn {

class Molecule;
typedef PP< Molecule > PMolecule;


class Molecule : public Object{

private:
	typedef Object inherited;

public:
//		int nPoints ;  // the number of points on the surface of the molecule
		PLearn::Mat chem ; // containts the chemical properties for each point on the surface
		PLearn::Mat geom ;     // contains the x,y,z position for each point on the surface
        string vrml_file ; 

        Molecule(); 
        Molecule(PLearn::Mat _chem, PLearn::Mat _geom, string vrml_file="") ; 

		// virtual ~Molecule() {}

private: 
  //! This does the actual building. 
  void build_();

protected: 
  //! Declares this class' options.
  static void declareOptions(OptionList& ol);

public:
  // Declares other standard object methods.
  PLEARN_DECLARE_OBJECT(Molecule);

  // simply calls inherited::build() then build_() 
  virtual void build();

  //! Transforms a shallow copy into a deep copy
  virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);




		static std::string getVrmlType(const std::string& name) ; 
		static void readVrml(const std::string& name,PLearn::SurfMesh& sm) ; 
		static void writeVrml(const std::string& name,const std::string& to,PLearn::SurfMesh& sm) ; 
		static void getVrmlVertexCoords(const std::string& name,PLearn::Mat& xmat) ; 
        static void readMolecules(const std::string & fileName, std::vector<PMolecule> & storage ) ; 
		static PMolecule readMolecule(const std::string & fileName) ; 

}; 
// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(Molecule);

}


#endif
