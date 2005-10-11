#include <cstdio>
#include <boost/config.hpp>
#include "SurfaceMesh.h"
#include "geometry.h"

using namespace PLearn;
using namespace std;
using namespace boost;


///////////////////////////////////////////


int main( int argc, char** argv )
{
  try
  {

/*
    SurfMesh sm = new SurfaceMesh();

    sm->readVRMLIndexedFaceSet
      ( "/u/lamblinp/work/test/vrml/bidon.vrml" );


    Mat rot = rotationFromFixedAngles( 15, 0, 0 );
    transformMesh( rot, Vec(3), sm );

    sm->writeVRMLIndexedFaceSet
      ( "/u/lamblinp/work/test/vrml/bidon_rot.vrml" );
*/



    cout << "rot = rotationFromFixedAngles( 0, 0, 15 ): " << endl;
    Mat rot = rotationFromFixedAngles( 0, 0, 15 );
    cout << rot << endl;

    cout << "ang = fixedAnglesFromRotation( rot ): " << endl;
    Vec ang = fixedAnglesFromRotation( rot );
    cout << ang << endl;

    cout << "uz = [ 0 0 1 ]:" << endl;
    Vec uz( 3 );
    uz << "0 0 1";
    cout << uz << endl;

    cout << "rot = rotationFromAxisAngle( uz, 15*DEG2RAD ):" << endl;
    rot << rotationFromAxisAngle( uz, 15*DEG2RAD );
    cout << rot << endl;

    cout << "ang = fixedAnglesFromRotation( rot ): " << endl;
    ang << fixedAnglesFromRotation( rot );
    cout << ang << endl;

cout << "-----------------" << endl;


    cout << "trans = [ 2 3 5 ]" << endl;
    Vec trans( 3 );
    trans << "2 3 5";
    cout << trans << endl;

    cout << "points: " << endl;
    string points_s = "0 0 0   1 0 0   1 1 0   0 1 1" ;
    Mat points( 4, 3 );
    points << points_s;
    cout << points << endl;

/*
    cout << "rpoints: " << endl;
    Mat rpoints( 4, 3 );
    productTranspose( rpoints, points, rot );
    cout << rpoints << endl;
*/

/*
    cout << "tpoints: " << endl;
    Mat tpoints( 4, 3 );
    tpoints = points + trans;
*/

    cout << "tpoints: " << endl;
    Mat tpoints( 4, 3 );
    transformPoints( rot, trans, points, tpoints );
    cout << tpoints << endl;

//    cout << "weights = [ 0 .33 .33 .34 ]:" << endl;
    cout << "weights = Vec( 4, .25 ):" << endl;
    Vec weights( 4, .25 );
    cout << weights << endl;

    real error=0;

    cout << "weightedTranformationFromMatchedPoints( tpoints, points, weights, rot, trans, error ):" << endl;
    weightedTransformationFromMatchedPoints( tpoints, points, weights, rot, trans, error ) ; 
    cout << "rot = " << endl << rot << endl;
    cout << "trans = " << trans << endl;
    cout << "error = " << error << endl;

cout << "------" << endl;

/*
    cout << "ang = fixedAnglesFromRotation( rot ): " << endl;
    ang << fixedAnglesFromRotation( rot );
    cout << ang << endl;

    cout << "error = " << error << endl;

    cout << "rot = rotationFromFixedAngles( ang[0], ang[1], ang[2] ): " << endl;
    rot << rotationFromFixedAngles( ang[0], ang[1], ang[2] );
    cout << rot << endl;
*/
    cout << "fpoints: " << endl;
    Mat fpoints( 4, 3 );
    transformPoints( rot, trans, tpoints, fpoints );
    cout << fpoints << endl;

cout << "-----------------" << endl;


/*
    cout << "weightedTransformationFromMatchedPoints( points, rpoints, weights, rot, trans, error ):" << endl;
    Vec trans( 3 );
    weightedTransformationFromMatchedPoints( points, rpoints, weights, rot, trans, error );
    cout << "rot = " << endl << rot << endl;
    cout << "trans = " << trans << endl;

    cout << "ang = fixedAnglesFromRotation( rot ): " << endl;
    ang << fixedAnglesFromRotation( rot );
    cout << ang << endl;

    cout << "error = " << error << endl;

    cout << "fpoints: " << endl;
    productTranspose( fpoints, rpoints, rot );
    fpoints += trans;
    cout << fpoints << endl;
*/

  }
  catch( PLearnError e )
  {
    cout << e.message() << endl;
  }
  return 0;
}

