// -*- C++ -*-

// MoleculeTemplateLearner.cc
//
// Copyright (C) 2005 Dan Popovici 
// 
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
 * $Id: .pyskeleton_header 544 2003-09-01 00:05:31Z plearner $ 
 ******************************************************* */

// Authors: Dan Popovici

/*! \file MoleculeTemplateLearner.cc */


#include "MoleculeTemplateLearner.h"
#include "WeightedLogGaussian.h"
#include <plearn/var/ConcatRowsVariable.h>
#include <plearn/var/LiftOutputVariable.h>
#include <plearn/var/NegCrossEntropySigmoidVariable.h>
#include <plearn/var/BinaryClassificationLossVariable.h>
#include <plearn/var/ProductVariable.h>
#include <plearn/var/TimesConstantVariable.h>
#include <plearn/var/Var_utils.h>
#include <plearn/var/MaxVariable.h>
#include "NoBpropVariable.h"

#include <plearn/var/Var.h>
#include <plearn/var/Var_all.h>
#include "plearn/display/DisplayUtils.h"
//#include "linearalign.h"

namespace PLearn {
    using namespace std;

void displayVarGr(const Var& v, bool display_values)
{
        displayVarGraph(v,display_values,0);
}
                                                                                                                                                             
void displayVarFn(const Func& f,bool display_values)
{
        displayFunction(f,display_values,0);
}


    MoleculeTemplateLearner::MoleculeTemplateLearner() : 
        nhidden(10) , 
        weight_decay(0),
        noutputs(1),
        batch_size(1),
        scaling_factor(1),
        lrate2(1),
	    training_mode(true),
	      builded(false)
            /* ### Initialize all fields to their default value here */
    {
        // load the molecules in a vector

        // ### You may or may not want to call build_() to finish building the object
        // build_();
    }

    PLEARN_IMPLEMENT_OBJECT(MoleculeTemplateLearner, "ONE LINE DESCRIPTION", "MULTI-LINE \nHELP");

    void MoleculeTemplateLearner::declareOptions(OptionList& ol)
    {
        // ### Declare all of this object's options here
        // ### For the "flags" of each option, you should typically specify  
        // ### one of OptionBase::buildoption, OptionBase::learntoption or 
        // ### OptionBase::tuningoption. Another possible flag to be combined with
        // ### is OptionBase::nosave

        // ### ex:
        declareOption(ol, "nhidden", &MoleculeTemplateLearner::nhidden, OptionBase::buildoption,
                "Number of hidden units in first hidden layer (0 means no hidden layer)\n");

        declareOption(ol, "weight_decay", &MoleculeTemplateLearner::weight_decay, OptionBase::buildoption,
                "weight_decay, preaty obvious right :) \n");

        declareOption(ol, "batch_size", &MoleculeTemplateLearner::batch_size, OptionBase::buildoption,
                "How many samples to use to estimate the average gradient before updating the weights\n"
                "0 is equivalent to specifying training_set->length() \n");
  
        declareOption(ol, "optimizer", &MoleculeTemplateLearner::optimizer, OptionBase::buildoption, 
                "Specify the optimizer to use\n");

        declareOption(ol, "n_active_templates", &MoleculeTemplateLearner::n_active_templates, OptionBase::buildoption, 
                "Specify the index of the molecule to use as seed for the actives\n");
        
        declareOption(ol, "n_inactive_templates", &MoleculeTemplateLearner::n_inactive_templates, OptionBase::buildoption, 
                "Specify the index of the molecule to use as seed for the inactives\n");
        
        declareOption(ol, "lrate2", &MoleculeTemplateLearner::lrate2, OptionBase::buildoption, 
                "The lrate2\n");

		declareOption(ol, "training_mode", &MoleculeTemplateLearner::training_mode, OptionBase::buildoption, 
				"training_mode\n");

		declareOption(ol, "templates", &MoleculeTemplateLearner::templates, OptionBase::learntoption, 
				"templates\n");

		declareOption(ol, "paramsvalues", &MoleculeTemplateLearner::paramsvalues, OptionBase::learntoption, 
				"paramsvalues\n");
		


        
        
        // Now call the parent class' declareOptions
        inherited::declareOptions(ol);
    }

    
    
    void MoleculeTemplateLearner::build_()
    {
      if ((train_set || !training_mode) && !builded){

      builded = true ; 

			n_templates = n_active_templates + n_inactive_templates ; 

			vector<int>  id_templates ; 

			if (training_mode) {
				Molecules.clear() ;  
				Molecule::readMolecules("g1active.txt",Molecules) ; //TODO : make filelist1 an option
				n_actives = Molecules.size() ; 
				Molecule::readMolecules("g1inactive.txt",Molecules) ; 

//				n_inactives = Molecules.size() - n_actives ;  // TODO : is needed ??

				
				set<int> found ; 
				Vec t(2) ; 

				// find the ids for the active templates 

				int nr_find_active = n_active_templates ; 
				for(int i=0 ; i<train_set.length() ; ++i) { 
					train_set -> getRow( i , t); 
					if (nr_find_active > 0 && t[1] == 1 && found.count((int)t[0])==0 ){
						nr_find_active -- ; 
						id_templates.push_back((int)t[0]);                        
						found.insert((int)t[0]);
					}
					if (nr_find_active == 0) break ; 
				}    

				if (nr_find_active > 0){
					PLERROR("There are not enought actives in the dataset") ; 
				}

				int nr_find_inactive = n_inactive_templates ; 
				for(int i=0 ; i<train_set.length() ; ++i) { 
					train_set -> getRow( i , t); 
					if (nr_find_inactive > 0 && t[1] == 0 && found.count((int)t[0])==0 ){
						nr_find_inactive -- ; 
						id_templates.push_back((int)t[0]);                        
						found.insert((int)t[0]);
					}
					if (nr_find_inactive == 0) break ; 
				}    

				if (nr_find_inactive > 0){
					PLERROR("There are not enought inactives in the dataset") ; 
				}

			}                        
            
            input_index = Var(1,"input_index") ; 
            
            mu.resize(n_templates) ;
            sigma.resize(n_templates) ; 
            sigma_square.resize(n_templates) ;
            S.resize(n_templates) ; 
            

            templates.resize(n_templates)  ; 

            for(int i=0 ; i<n_templates ; ++i) { 

				if (training_mode) {
					mu[i] = Var(Molecules[id_templates[i]]->chem.length() , Molecules[id_templates[i]]->chem.width() , "Mu") ; 
					mu[i]->matValue << Molecules[id_templates[i]]->chem ;  

					sigma[i] = Var(Molecules[id_templates[i]]->chem.length() , Molecules[id_templates[i]]->chem.width() , "Sigma") ; 
					sigma[i]->value.fill(0) ; 

				}
				else {				
					mu[i] = Var(templates[i]->chem.length() , templates[i]->chem.width() ,"Mu" ) ; 					
					sigma[i] = Var(templates[i]->dev.length() , templates[i]->dev.width() , "Sigma") ; 
				}
                
                params.push_back(mu[i]) ;
                params.push_back(sigma[i]) ;

                if (training_mode)
                    sigma_square[i] = new ExpVariable(sigma[i]) ; 
                else
                    sigma_square[i] = sigma[i] ; 

                    

//				if (!training_mode) {
//					sigma_square[i]->value.fill(1) ; 
//				}
				
                if (training_mode) {

				templates[i] = new Template() ; 
                templates[i]->chem.resize(mu[i]->matValue.length() , mu[i]->matValue.width()) ; 
                templates[i]->chem << mu[i]->matValue ; 
                
                templates[i]->geom.resize(Molecules[id_templates[i]]->geom.length() , Molecules[id_templates[i]]->geom.width()) ;                 
                templates[i]->geom << Molecules[id_templates[i]]->geom ;                 
                templates[i]->vrml_file = Molecules[id_templates[i]]->vrml_file ; 
                templates[i]->dev.resize (sigma_square[i]->matValue.length() ,  sigma_square[i]->matValue.width() ) ; 
                templates[i]->dev << sigma_square[i]->matValue ;  // SIGMA_SQUARE has not the right value yet ??????

				}
            
            }
                        
            for(int i=0 ; i<n_templates ; ++i) 
    					S[i] = new WeightedLogGaussian(training_mode , i, input_index, mu[i], sigma_square[i] , templates[i]) ; 

            
            V = Var(nhidden , n_templates , "V") ; 
            V_b = Var(nhidden , 1 , "V_b") ;
//            V_direct = Var(1 , 2  , "V_direct") ; 
            
            mu_S.resize(n_templates) ; 
            sigma_S.resize(n_templates) ; 
            sigma_square_S.resize(n_templates) ; 


            S_after_scaling.resize(n_templates) ; 
            
            sigma_s_vec.resize(n_templates) ; 
            
            for(int i=0 ; i<n_templates ; ++i) { 
                mu_S[i] = Var(1 , 1) ;                 
                sigma_S[i] = Var(1 , 1) ;
                if (training_mode)
                    sigma_square_S[i] = new SquareVariable(sigma_S[i]) ;
                else
                    sigma_square_S[i] = sigma_S[i] ; 

                params.push_back(mu_S[i]);
                params.push_back(sigma_S[i]);
                S_after_scaling[i] = new DivVariable(S[i] - mu_S[i] , sigma_square_S[i] ) ; 
            }
            
            S_std.resize(n_templates)  ; 
            
          
            for(int i=0 ; i<n_templates ; ++i) { 
                
                S_after_scaling[i] = new NoBpropVariable (S_after_scaling[i] , &S_std[i] ) ;
                
            }

            
            temp_S = new ConcatRowsVariable(S_after_scaling) ; 
            hl = tanh(product(V,temp_S) + V_b) ; 
            
            params.push_back(V);
            params.push_back(V_b);
//          params.push_back(V_direct);

            W = Var(1, nhidden) ; 

            W_b = Var(1 , 1) ; 
            
            y_before_transfer = (product(W,hl) + W_b); //+product(V_direct , temp_S)) ;
            y = sigmoid(y_before_transfer) ;
            
            
            params.push_back(W);
    
            penalties.append(affine_transform_weight_penalty(V, (weight_decay), 0, "L1"));

            params.push_back(W_b);

			// initialize all the parameters
			if (training_mode) {

				paramsvalues.resize(params.nelems());

				for(int i=0 ; i<n_templates ; ++i) { 
					mu_S[i]->value.fill(0) ; 
					sigma_S[i]->value.fill(1) ; 
				}
				Vec t_mean(n_templates) , t_std(n_templates) ; 

				compute_S_mean_std(t_mean,t_std) ;        

				for(int i=0 ; i<n_templates ; ++i) { 
					mu_S[i]->value[0] = t_mean[i] ;         
					sigma_S[i]->value[0] = sqrt(t_std[i]) ; 
				}

				for(int i=0 ; i<n_templates ; ++i) { 
					S_std[i] = lrate2 ; 
				}

				manual_seed(seed_) ;

				fill_random_uniform(V->matValue,-1,1) ;         
				fill_random_uniform(V_b->matValue,-1,1) ;                 
				//        fill_random_uniform(V_direct->matValue,-0.0001,0.0001) ;                 
				fill_random_uniform(W->matValue,-1,1) ; 
				fill_random_uniform(W_b->matValue,-1,1) ; 


			}
			else {			
		                params << paramsvalues;
			}

			params.makeSharedValue(paramsvalues);

            if (!training_mode) {

                for(int i=0 ; i<n_templates ; ++i) { 
                    sigma_S[i]->value[0] *= sigma_S[i]->value[0] ; 
                }
                
                for(int i=0 ; i<n_templates ; ++i) {                     
                    for(int j=0 ; j<sigma_square[i]->matValue.length() ; ++j) { 
                        for(int k=0 ; k<sigma_square[i]->matValue.width() ; ++k) { 
                            sigma_square[i]->matValue[j][k] = exp(sigma[i]->matValue[j][k]) ; 
                        }
                    }
                }
            }

/*            
            for(int i=0 ; i<n_templates ; ++i) {     
                sigma_s_vec[i] = sigma_S[i]->value[0] ; 
            }
*/            
            

            target = Var(1 , "the target") ; 

            costs.resize(3) ; 
            
            costs[0] = stable_cross_entropy(y_before_transfer , target) ; 
            costs[1] = binary_classification_loss(y, target);
            costs[2] = lift_output(y , target);
            
            
            f_output = Func(input_index, y) ; 
//            displayVarFn(f_output , 0) ; 
            

            training_cost = hconcat(sum(hconcat(costs[0] & penalties)));
            training_cost->setName("training cost");

            test_costs = hconcat(costs);
            test_costs->setName("testing cost");
            
            output_target_to_costs = Func(y & target , test_costs) ; 

            test_costf = Func(input_index & target , y & test_costs);

        }
    }

    // ### Nothing to add here, simply calls build_
    void MoleculeTemplateLearner::build()
    {
        inherited::build();
        build_();
    }


    void MoleculeTemplateLearner::makeDeepCopyFromShallowCopy(CopiesMap& copies)
    {
        inherited::makeDeepCopyFromShallowCopy(copies);

        // ### Call deepCopyField on all "pointer-like" fields 
        // ### that you wish to be deepCopied rather than 
        // ### shallow-copied.
        // ### ex:
        // deepCopyField(trainvec, copies);
  
          varDeepCopyField(input_index,copies) ; 
          deepCopyField(mu , copies) ; 
          deepCopyField(sigma , copies) ; 
          deepCopyField(mu_S , copies) ; 
          deepCopyField(sigma_S , copies) ; 
          deepCopyField(sigma_square_S , copies) ; 
          deepCopyField(sigma_square , copies) ; 
          deepCopyField(S , copies) ; 
          deepCopyField(S_after_scaling, copies) ; 
          deepCopyField(params , copies) ; 
          deepCopyField(penalties , copies) ; 
          
          varDeepCopyField(V,copies) ; 
          varDeepCopyField(W,copies) ; 
          varDeepCopyField(V_b,copies) ; 
          varDeepCopyField(W_b,copies) ; 
          varDeepCopyField(V_direct,copies) ; 
          varDeepCopyField(hl,copies) ; 
          varDeepCopyField(y,copies) ; 
          varDeepCopyField(y_before_transfer,copies) ; 
          varDeepCopyField(training_cost,copies) ; 
          varDeepCopyField(test_costs,copies) ; 
          varDeepCopyField(target,copies) ; 
          varDeepCopyField(temp_S,copies) ; 
          
          deepCopyField(costs , copies) ; 
          deepCopyField(temp_output , copies) ; 
          deepCopyField(S_std , copies) ; 
          deepCopyField(sigma_s_vec , copies) ; 
          deepCopyField(f_output , copies) ; 
          deepCopyField(output_target_to_costs , copies) ; 
          deepCopyField(test_costf , copies) ; 
          deepCopyField(optimizer , copies) ; 
          deepCopyField(templates , copies) ; 
          deepCopyField(paramsvalues , copies) ; 
          
    
    }


    int MoleculeTemplateLearner::outputsize() const
    {
        // Compute and return the size of this learner's output (which typically
        // may depend on its inputsize(), targetsize() and set options).
        return noutputs ; 

    }

    void MoleculeTemplateLearner::forget()
    {
        //! (Re-)initialize the PLearner in its fresh state (that state may depend on the 'seed' option)
        //! And sets 'stage' back to 0   (this is the stage of a fresh learner!)
        /*!
          A typical forget() method should do the following:
          - initialize a random number generator with the seed option
          - initialize the learner's parameters, using this random generator
          - stage = 0
         */
        initializeParams() ; 
    }
    
    void MoleculeTemplateLearner::compute_S_mean_std(Vec & t_mean , Vec & t_std){
        
        int l = train_set->length() ; 

        Vec current_S(n_templates) ; 
        Func computeS(input_index , temp_S ) ;         
        
        Mat valueS(l,n_templates) ; 
        Vec training_row(2) ;         
        Vec current_index(1) ; 

        t_mean.fill(0) ; 
        t_std.fill(0) ; 
            
        
        computeS->recomputeParents();
        
        FILE * f = fopen("nicolas.txt","wt") ; 
        
        for(int i=0 ; i<l ; ++i) { 
            
            train_set->getRow(i , training_row) ; 
            current_index[0] = training_row[0] ; 
			
            for(int j=0 ; j<n_templates ; ++j) { 
                PP<WeightedLogGaussian> ppp = dynamic_cast<WeightedLogGaussian*>( (Variable*) S[j]); //->molecule = Molecules[(int)training_row[0]] ; 
                ppp->molecule = Molecules[(int)training_row[0]] ; 
            }

            computeS->fprop(current_index , current_S ) ;             
            
            for(int j=0 ; j<n_templates ; ++j) { 
                valueS[i][j] = current_S[j] ; 
                t_mean[j] += current_S[j] ; 
                cout << i << " " << current_S[j]  << endl ; 
            }
            fprintf( f , "%f %f %d\n" ,  current_S[0] , current_S[1] , training_row[1] > 0 ? 1 : -1 ) ; 
            
        }
        fclose(f) ; 

        for(int i=0 ; i<n_templates ; ++i) { 
            t_mean[i]/= l  ; t_mean[i]/=l ; 
        }

        for(int i=0 ; i<l ; ++i) {
            for(int j=0 ; j<n_templates ; ++j) { 
                t_std[j] += square(valueS[i][j] - t_mean[j]) ; 
            }
        }

        for(int i=0 ; i<n_templates ; ++i) { 
            t_std[i] /= l ; 
            t_std[i] = sqrt(t_std[i]) ; 
        }

    }
    void MoleculeTemplateLearner::train()
    {
        if(!train_stats)  // make a default stats collector, in case there's none
            train_stats = new VecStatsCollector();

        int l = train_set->length();

        int nsamples = 1;

        Func paramf = Func(input_index & target, training_cost); // parameterized function to optimize


        Var totalcost = meanOf(train_set, paramf, nsamples);
        if(optimizer)
        {
            optimizer->setToOptimize(params, totalcost);
            optimizer->build();
            optimizer->reset();
        }
        else PLERROR("EntropyContrastLearner::train can't train without setting an optimizer first!");
        ProgressBar* pb = 0;
        if(report_progress>0) {
            pb = new ProgressBar("Training MoleculeTemplateLearner stage " + tostring(stage) + " to " + tostring(nstages), nstages-stage);
        }



//        int optstage_per_lstage = l/nsamples;

               
        while(stage<nstages)
        {
            optimizer->nstages = 1 ; // optstage_per_lstage;
            double mean_error = 0.0 ; 

            for(int k=0 ; k<train_set->length() ; ++k) { 


                
                //update the template 
                for(int i=0 ; i<n_templates ; ++i) 
                {
                    templates[i]->chem << mu[i]->matValue ; 

                    templates[i]->dev << sigma_square[i]->matValue ; 

                }
                //align only the next training example
                Mat temp_mat ;                 
                Vec training_row(2) ; 
				train_set->getRow(k , training_row) ; 
                for(int i=0 ; i<n_templates ; ++i) { 
                        
//						string s =  train_set->getString(k,0) ; 
//                        performLP(Molecules[(int)training_row[0]],templates[i], temp_mat , false) ; 
//                        W_lp[i][(int)training_row[0]]->matValue << temp_mat ; 
			        PP<WeightedLogGaussian> ppp = dynamic_cast<WeightedLogGaussian*>( (Variable*) S[i]); //->molecule = Molecules[(int)training_row[0]] ; 
                    ppp->molecule = Molecules[(int)training_row[0]] ; 
//					S[i]->molecule = Molecules[(int)training_row[0]] ; 
                }

                // clear statistics of previous epoch
                train_stats->forget();

//                displayVarFn(f_output , true) ; 
				
                optimizer->optimizeN(*train_stats);
//                temp_S->verifyGradient(1e-4) ; 

                train_stats->finalize(); // finalize statistics for this epoch
                cout << "Example " << k << " train objective: " << train_stats->getMean() << endl;
                mean_error += train_stats->getMean()[0] ; 
                
                if(pb)
                    pb->update(stage);
            }
           
            
            cout << endl << endl <<"Epoch " << stage << " mean error " << mean_error/l << endl << endl; 

            
            ++stage;
            


        }        
/*
        Mat temp_mat ;                 
        for(int i=0 ; i<n_templates ; ++i) { 
            W_lp[i].resize(Molecules.size()) ; 
            for(unsigned int j=0 ; j<Molecules.size() ; ++j) { 
                performLP(Molecules[j],templates[i], temp_mat , false) ; 
                W_lp[i][j]->matValue << temp_mat ; 
            }
        }
*/
        for(int i=0 ; i<n_templates ; ++i) {         
            cout << "mu[0]" << mu[i]->matValue << endl ; 
            cout << "sigma[0]" << sigma_square[i]->matValue << endl ; 
        }


        output_target_to_costs->recomputeParents();
        test_costf->recomputeParents();

//		molecule = NULL ; 

    }


    void MoleculeTemplateLearner::computeOutput(const Vec& input, Vec& output) const
    {
        output.resize(1);
        f_output->fprop(input,output) ;

    }    

    void MoleculeTemplateLearner::computeCostsFromOutputs(const Vec& input, const Vec& output, 
            const Vec& target, Vec& costsv) const
    {
        PLERROR("You are not allowed to reach this function :((((") ; 
        // Compute the costs from *already* computed output. 
        output_target_to_costs->fprop(output & target , costsv) ; 
    }                                

    void MoleculeTemplateLearner::computeOutputAndCosts(const Vec& inputv, const Vec& targetv,
            Vec& outputv, Vec& costsv) const
    {
        test_costf->fprop(inputv&targetv, outputv&costsv);
    }
   
    TVec<string> MoleculeTemplateLearner::getTestCostNames() const
    {
        // Return the names of the costs computed by computeCostsFromOutpus
        // (these may or may not be exactly the same as what's returned by getTrainCostNames).
        // ...

        //TODO : put some code here 
        TVec<string> t(3) ; 
        t[0] = "NLL" ; 
        t[1] = "binary_class_error" ;
        t[2]  = "lift_output" ;
        return t ; 
    }

    TVec<string> MoleculeTemplateLearner::getTrainCostNames() const
    {
        TVec<string> t(3) ; 
        t[0] = "NLL" ; 
        t[1] = "binary_class_error" ;
        t[2] = "lift_output" ; 
        return t ; 
    }

    void MoleculeTemplateLearner::initializeParams(){
       

    }
    void MoleculeTemplateLearner::test(VMat testset, PP<VecStatsCollector> test_stats, 
                      VMat testoutputs, VMat testcosts)const {
        for(int i=0 ; i<n_templates ; ++i) { 
                PP<WeightedLogGaussian> ppp = dynamic_cast<WeightedLogGaussian*>( (Variable*) S[i]); //->molecule = Molecules[(int)training_row[0]] ; 
                ppp->test_set = testset ; 
        }

        inherited::test(testset , test_stats , testoutputs , testcosts) ; 
    }


} // end of namespace PLearn
