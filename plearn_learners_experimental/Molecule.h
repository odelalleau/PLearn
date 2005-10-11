
#ifndef Molecule_INC
#define Molecule_INC


#include <plearn/io/load_and_save.h>
#include <plearn/math/TMat_maths.h>
#include "SurfaceMesh.h"
#include "geometry.h"
#include <vector>

#include <string>



class Molecule{

	public:
		int nPoints ;  // the number of points on the surface of the molecule
		PLearn::Mat chem ; // containts the chemical properties for each point on the surface
		PLearn::Mat geom ;     // contains the x,y,z position for each point on the surface

        Molecule(); 
        Molecule(PLearn::Mat _chem, PLearn::Mat _geom) ; 
        Molecule(const Molecule & m) ; 
        Molecule & operator=(const Molecule  & m){

            chem.resize(m.chem.length(), m.chem.width() ) ; 
            chem << m.chem ; 

            geom.resize(m.geom.length(), m.geom.width() ) ; 
            geom << m.geom ; 

            return *this ; 
        }

		static std::string getVrmlType(const std::string& name) ; 
		static void readVrml(const std::string& name,PLearn::SurfMesh& sm) ; 
		static void writeVrml(const std::string& name,const std::string& to,PLearn::SurfMesh& sm) ; 
		static void getVrmlVertexCoords(const std::string& name,PLearn::Mat& xmat) ; 
        static void readMolecules(const std::string & fileName, std::vector<Molecule> & storage ) ; 

}; 


#endif
