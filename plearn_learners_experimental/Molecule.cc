#include "Molecule.h"
#include <plearn/db/getDataSet.h>
#include <plearn/vmat/VMat.h>

using namespace std ; 
using namespace PLearn ; 
                                                                                                                                                  
Molecule::Molecule(){}
Molecule::Molecule(Mat _chem, Mat _geom) {
    chem.resize(_chem.length(), _chem.width() ) ; 
    chem << _chem ; 
     
    geom.resize(_geom.length(), _geom.width() ) ; 
    geom << _geom ; 
    
}

Molecule::Molecule(const Molecule & m) {
    
    chem.resize(m.chem.length(), m.chem.width() ) ; 
    chem << m.chem ; 
     
    geom.resize(m.geom.length(), m.geom.width() ) ; 
    geom << m.geom ; 
    
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
void Molecule::readMolecules(const string & fileName, vector<Molecule> & storage ) {

	ifstream f(fileName.c_str());
	string file;
	Mat chem, geom ; 

    Vec column_indices(3) ; 
    for(int i=0 ; i<3 ; ++i) column_indices[i] = i ; 
    

	while(f>>file){
//		load(file+"SurfacePrpi.mat",chem);
        VMat t = getDataSet(file+".amat") ; 
        Mat full_chem = t.toMat() ; 
        chem.resize(full_chem.length() ,3 ) ; 
        selectColumns(full_chem , column_indices , chem) ; 
        normalize(chem) ;         
		Molecule::getVrmlVertexCoords(file,geom);
        Molecule m(chem,geom) ; 
		storage.push_back(m) ; 
	}
    
}

