// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2001-2002 Nicolas Chapados, Ichiro Takeuchi, Jean-Sebastien Senecal
// Copyright (C) 2002 Xiangdong Wang, Christian Dorion

// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
//  1. Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
// 
//  2. Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.
// 
//  3. The name of the authors may not be used to endorse or promote
//     products derived from this software without specific prior written
//     permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
// NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// This file is part of the PLearn library. For more information on the PLearn
// library, go to the PLearn Web site at www.plearn.org


/* *******************************************************      
   * $Id: WeightedLogGaussian.cc 2052 2004-07-19 22:31:11Z Dan Popovici $
   * This file is part of the PLearn library.
   ******************************************************* */

#include "WeightedLogGaussian.h"
#include <plearn/var/Var_utils.h>
#include "Molecule.h"
#include "linearalign.h"

namespace PLearn {
	using namespace std;


/** WeightedLogGaussian **/

	PLEARN_IMPLEMENT_OBJECT(WeightedLogGaussian,
							"Variable that computes log P(X | C_k) for the MoleculeTemplateLearner",
							"-1/2 \\sum_{t,t'}w_{tt'}.....\n"
						   );

	WeightedLogGaussian::WeightedLogGaussian(bool the_training_mode , int the_class_label, Var input_index, Var mu, Var sigma, MoleculeTemplate the_template)
	: inherited(input_index & mu & sigma, 1 , 1)
	{
		build_();
		class_label = the_class_label ; 		
		current_template = the_template ; 
//		molecule = new Molecule() ; 
		training_mode = the_training_mode ; 
	}

	void
	WeightedLogGaussian::build()
	{
		inherited::build();
		build_();
	}

	void
	WeightedLogGaussian::build_()
	{
	}

	void WeightedLogGaussian::recomputeSize(int& l, int& w) const
	{
		if (varray.size()) {
			l = 1;
			w = 1;
		} else
			l = w = 0;
	}

	void WeightedLogGaussian::fprop()
	{
		real ret = 0.0 ; 
		int p = mu()->width() ;
	    int training_index = input_index()->value[0] ; 

		if (! training_mode )  // read in the file only once
		{		
			string filename = test_set->getValString(0, input_index()->value[0]) ;  			
			molecule = Molecule::readMolecule(filename) ; 
		}


		performLP(molecule,current_template, W_lp , false) ; 
		int n = W_lp.width() ; 
		int m = W_lp.length() ; 
    
//    printf("(%d %d) -> length allignment = (%d %d) \n" ,training_index , class_label,n,m) ; 

		Mat input ;
		input = molecule->chem ; 

		for (int i=0 ; i<n ; ++i) {
			for (int j=0 ; j<m ; ++j) {
				for (int k=0 ; k<p ; ++k) {
					ret += W_lp[j][i] * square((input[j][k] - mu()->matValue[i][k]))/square(sigma()->matValue[i][k]) ; 
				}
			}
		}

		ret *= - 0.5 ; 

		for (int i=0 ; i<n ; ++i) {
			for (int k=0 ; k<p ; ++k) {
				ret -= log(sigma()->matValue[i][k]) ;
//             if (sigma()->matValue[i][k] > 20){
//                 cout << sigma()->matValue[i][k] ; 
//                 cout << ret ;                 
//             }
			}
		}


		if (isnan(ret))
			PLERROR("NAN") ;

		valuedata[0] = ret ; 
	}

	inline double cube(double x){
		return x*x*x ; 
	}

	void WeightedLogGaussian::bprop()
	{
		int n = mu()->length() ; 
		int p = mu()->width() ;
//		Mat W_lp ; 
		Mat input ;
//		int training_index = input_index()->value[0] ; 

		input = (molecule)->chem ;
		

		int m = W_lp.length() ; 
		
		
//    cout << "MATGRDIENT" << class_label<< endl ; 


		for (int i=0 ; i<n ; ++i) {
			for (int k=0 ; k<p ; ++k) {
				sigma()->matGradient[i][k] -= gradientdata[0] / sigma()->matValue[i][k] ; 
				for (int j=0 ; j<m ; ++j) {
					mu()->matGradient[i][k] += gradientdata[0] * W_lp[j][i] * (input[j][k] - mu()->matValue[i][k]) / square(sigma()->matValue[i][k]) ;
					sigma()->matGradient[i][k] += gradientdata[0] * W_lp[j][i] * square(input[j][k] - mu()->matValue[i][k]) / cube(sigma()->matValue[i][k]) ;

					if (fabs(sigma()->matGradient[i][k]) > 5) {
//                 char buf[20] ; 
//                 fprintf(buf , "error-%d-%d",, )  ; 
//                 FILE * fo = fopen("error.txt","wt")  ; 
//                 fprintf(fo , "gradientdata[0] = %lf \n" , gradientdata[0]) ; 
//                 fprintf(fo , " W_lp[training_index]->matValue[j][i]= %lf \n" , W_lp[training_index]->matValue[j][i]) ; 
//                 fprintf(fo , " input[j][k] = %lf \n" , input[j][k]) ; 
//                 fprintf(fo , " mu()->matValue[i][k] = %lf \n" , mu()->matValue[i][k]) ; 
//						printf(" sigma()->matValue[i][k] = %lf \n" , sigma()->matValue[i][k]) ; 
//                 fclose(fo) ;
//                 exit(1) ; 
					}


				}
//            cout << sigma()->matValue[i][k] << endl ; 
			}
		}
		cout << "Grad Data "<< gradientdata[0] << endl ; 
		//    cout << "sigmagrad " <<sigma()->matGradient << endl ;
		//      sigma()->matGradiant =     

	}
	void WeightedLogGaussian::symbolicBprop()
	{
// input->accg(g * (input<=threshold));
	}

} // end of namespace PLearn


