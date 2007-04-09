#include "Molecule.h"
#include <plearn/db/getDataSet.h>
#include <plearn/vmat/VMat.h>

using namespace std ; 
namespace PLearn {

                                                                                                                                                  
Molecule::Molecule(){}
Molecule::Molecule(Mat _chem, Mat _geom, string vrml_file) {
    chem.resize(_chem.length(), _chem.width() ) ; 
    chem << _chem ; 
     
    geom.resize(_geom.length(), _geom.width() ) ; 
    geom << _geom ; 
    this->vrml_file = vrml_file ; 
    
}

PLEARN_IMPLEMENT_OBJECT(Molecule,
    "A Molecule",
    ""
);

void Molecule::declareOptions(OptionList& ol)
{
  declareOption(ol, "chem", &Molecule::chem, OptionBase::buildoption,
                "Chemical Properties");

  declareOption(ol, "geom", &Molecule::geom,
                OptionBase::buildoption,
                "Geom Properties");
  
  declareOption(ol, "vrml_file", &Molecule::vrml_file,
                OptionBase::buildoption,
                "The vrml filename");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

void Molecule::build_()
{}

void Molecule::build()
{
  inherited::build();
  build_();
}

void Molecule::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  deepCopyField(chem, copies);
  deepCopyField(geom, copies);

}

string Molecule::getVrmlType(const string& name) {
        string name2 = name + ".vrml";
        ifstream os(name2.c_str());
        string temp,s;
        s="";
        while(os>>temp){
                s=s+temp;
        }
        string::size_type a = s.find("Line");
        if (a!= string::npos) return string("line");
        a = s.find("Face");
        if (a!=string::npos) return string("face");
        PLERROR("getType : Invalid VRML file");
        return "";
}

void Molecule::readVrml(const string& name,SurfMesh& sm){
        string type = getVrmlType(name);
        if(type=="line"){
                sm->readVRMLIndexedLineSet(name+".vrml");
        }else if(type=="face"){
                sm->readVRMLIndexedFaceSet(name+".vrml");                
        }
}
void Molecule::writeVrml(const string& name,const string& to,SurfMesh& sm){
        string type = getVrmlType(name);
        if(type=="line"){
                sm->writeVRMLIndexedLineSet(to+".vrml");
        }else if(type=="face"){
                sm->writeVRMLIndexedFaceSet(to+".vrml");
        }

}
void Molecule::getVrmlVertexCoords(const string& name,Mat& xmat){
        SurfMesh xmesh = new SurfaceMesh();
        readVrml(name,xmesh);
        xmat = xmesh->getVertexCoords();
}

//append to storage the molecules from file 1
PMolecule Molecule::readMolecule(const string & file){


	Vec column_indices(5) ; 
	for(int i=0 ; i<5 ; ++i) column_indices[i] = i ; 


	Mat chem, geom ; 
	VMat t = getDataSet(file + ".amat") ; 
	Mat full_chem = t.toMat() ; 
	chem.resize(full_chem.length() ,5 ) ; 
	selectColumns(full_chem , column_indices , chem) ; 
	normalize(chem) ;         
	Molecule::getVrmlVertexCoords(file,geom);
//        Molecule m(chem,geom) ; 

	PMolecule pm = new Molecule(chem , geom ,file+".vrml" ) ; 
	return pm ;

}
void Molecule::readMolecules(const string & fileName, vector<PMolecule> & storage ) {

	ifstream f(fileName.c_str());
	string file;
    

	while(f>>file){
//		load(file+"SurfacePrpi.mat",chem);
		storage.push_back(readMolecule(file)) ; 		
	}
    
}

}

