// -*- C++ -*-

// IsomapTangentLearner.cc
//
// Copyright (C) 2004 Martin Monperrus 
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
 * $Id$ 
 ******************************************************* */

// Authors: Martin Monperrus

/*! \file IsomapTangentLearner.cc */


#include "Isomap.h"
#include "IsomapTangentLearner.h"
#include <plearn/ker/GeodesicDistanceKernel.h>
#include <plearn/ker/AdditiveNormalizationKernel.h>


namespace PLearn {
using namespace std;

IsomapTangentLearner::IsomapTangentLearner() : n_comp(2), knn(10)
/* ### Initialize all fields to their default value here */
{
    // ...

    // ### You may or may not want to call build_() to finish building the object
    // build_();
}

PLEARN_IMPLEMENT_OBJECT(IsomapTangentLearner, "Tangent learning based on Isomap Kernel", "MULTI-LINE \nHELP");

void IsomapTangentLearner::declareOptions(OptionList& ol)
{
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify  
    // ### one of OptionBase::buildoption, OptionBase::learntoption or 
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave
  
  
    declareOption(ol, "knn", &IsomapTangentLearner::knn, OptionBase::buildoption,
                  "Number of nearest neighbor taken into account");
    declareOption(ol, "n_comp", &IsomapTangentLearner::n_comp, OptionBase::buildoption,
                  "Number of Components");
    declareOption(ol, "iso_learner", &IsomapTangentLearner::iso_learner, OptionBase::learntoption,
                  "The Isomap Learner");
  
    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void IsomapTangentLearner::build_()
{
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation. 
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
    // ### You should assume that the parent class' build_() has already been called.
   
    iso_learner.knn = knn;
    iso_learner.n_comp = n_comp;
    if (train_set)
        iso_learner.setTrainingSet(train_set);
    iso_learner.build();

// peut etre qu'il faut un VectatsCollector

//       PP<VecStatsCollector> train_stats = new VecStatsCollector();
//     learner->setTrainStatsCollector(train_stats);
//     learner->setTrainingSet(trainset);
//     learner->train();

      
}

// ### Nothing to add here, simply calls build_
void IsomapTangentLearner::build()
{
    inherited::build();
    build_();
}


void IsomapTangentLearner::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields 
    // ### that you wish to be deepCopied rather than 
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("IsomapTangentLearner::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}


int IsomapTangentLearner::outputsize() const
{
    // Compute and return the size of this learner's output (which typically
    // may depend on its inputsize(), targetsize() and set options).
    return inputsize()*n_comp;
}

void IsomapTangentLearner::forget()
{
    //! (Re-)initialize the PLearner in its fresh state (that state may depend on the 'seed' option)
    //! And sets 'stage' back to 0   (this is the stage of a fresh learner!)
    /*!
      A typical forget() method should do the following:
      - initialize a random number generator with the seed option
      - initialize the learner's parameters, using this random generator
      - stage = 0
    */
}
    
void IsomapTangentLearner::train()
{
    // The role of the train method is to bring the learner up to stage==nstages,
    // updating train_stats with training costs measured on-line in the process.

    iso_learner.train();
    
}


void IsomapTangentLearner::computeOutput(const Vec& input, Vec& output) const
{
    // Compute the output from the input.
    // int nout = outputsize();
    // output.resize(nout);
    // ...
  
    // ici je recupere le GeodesicDistanceKernel
    PP<AdditiveNormalizationKernel> ank = dynamic_cast<AdditiveNormalizationKernel*>((Kernel*)iso_learner.kernel);  
    PP<GeodesicDistanceKernel> gdk = dynamic_cast<GeodesicDistanceKernel*>((Kernel*)ank->source_kernel);

//   output.resize(outputsize());
  
    Mat k_xi_x;
    //cout<<input<<endl;
    // on fait un knn+1 au cas ou on elve la premiere ligne un peu plus loin
    gdk->distance_kernel->computeNearestNeighbors(input, k_xi_x, knn+1);

    Mat k_xi_x_sorted;
  
    // we assume that the trainingset contains each exemple only one time
    // here we manage the case of computing tangent plane on a point of the training set
    if (k_xi_x(0,0) < 1e-9)
        k_xi_x_sorted = k_xi_x.subMatRows(1,k_xi_x.length() - 1);
    else
        k_xi_x_sorted = k_xi_x;
    //cout<<k_xi_x.subMatRows(1,knn);
    Mat result(n_comp,inputsize());
  
    Vec dkdx(inputsize()); //dk/dx
    Vec temp(inputsize());
    Vec term2(inputsize());
    Vec term1(inputsize());
  
//   Vec tangentVector(inputsize()); // = sum_i v_ik*dk(i)/dx
  
    int ngn;
    VMat trainset = ank->specify_dataset;
    int n_examples = trainset->length();
    Mat diK_dx(n_examples,inputsize());
  
    int i,j,nc;
    real D;

    term1<<0;  
    // real seuil = 1e-9;
    for(j=0;j<n_examples;++j)
    {
        ngn = gdk->computeNearestGeodesicNeighbour(j, k_xi_x_sorted);// ngn minimise la distance geodesique entre input et j
        trainset->getRow(ngn,temp);
        temp << (input-temp);
        D =  norm(temp) + gdk->geo_distances->get(j,ngn);
        //      cout<<D<<endl;
        // probleme resolu: il faut appeler gdk->distance_kernel->compute...
//       cout<<" "<<D*D<<" "<<gdk->evaluate_i_x_from_distances(j,k_xi_x_sorted) <<endl;
        //if (norm(temp) > seuil)
        term1 += D*(temp)/norm(temp);
    }
    term1/=n_examples;

    for(i=0;i<n_examples;++i)
    {

        // get the nearest neighbor
        ngn = gdk->computeNearestGeodesicNeighbour(i, k_xi_x_sorted); // ngn minimise la distance geodesique entre input et i
      
        trainset->getRow(ngn,temp);
        //cout<<i<<"="<<ngn<<"-"<<int(k_xi_x_sorted(ngn,1))<<" ";
        temp << (input-temp); // temp = x-xN
        //       cout<<gdk->evaluate_i_x(i,input,k_xi_x_sorted);
        //       cout<<norm(temp)<<endl;
        term2<<0;
        D =  norm(temp) + gdk->geo_distances->get(i,ngn);
        //if (norm(temp) > seuil) 
        term2 = D*(temp)/norm(temp);
//       else
//         term2.fill(0);
      

        //cout<<term2<<endl;
        //cout<<sum;
        diK_dx(i) << (term1 - term2); // exactement la formule de NIPS
        //cout<<diK_dx(i);
    }
  
    for(nc=0;nc<n_comp;++nc)
    {
        // compute the corresponding vector with the Nystrom formula
        // d ek / dx = 1/n sum_i dK/dX
    
        // initialisation
        temp<<(0);
        for(i=0;i<n_examples;++i)
        {
            temp += (iso_learner.eigenvectors(nc,i) * diK_dx(i));
        }
        // on ne normalise pas car c'est la direction vecteur qui nous interesse et pas sa norme
        // en plus on normalise tout a 1 dans matlab pour eviter les erreurs numériques.
//     result(nc)<<(temp/iso_learner.eigenvalues[nc]);
        result(nc)<<(temp);
    }    
    //cout<<result; 
    // toVec: a mettre dans l'aide
    output << result.toVec();
  
}



void IsomapTangentLearner::computeCostsFromOutputs(const Vec& input, const Vec& output, 
                                                   const Vec& target, Vec& costs) const
{
// Compute the costs from *already* computed output. 
// ...
}                                

TVec<string> IsomapTangentLearner::getTestCostNames() const
{
    // Return the names of the costs computed by computeCostsFromOutpus
    // (these may or may not be exactly the same as what's returned by getTrainCostNames).
    // ...
    return TVec<string>();
}
 
TVec<string> IsomapTangentLearner::getTrainCostNames() const
{
    // Return the names of the objective costs that the train method computes and 
    // for which it updates the VecStatsCollector train_stats
    // (these may or may not be exactly the same as what's returned by getTestCostNames).
    // ...
    return TVec<string>();
}


} // end of namespace PLearn


/*
  Local Variables:
  mode:c++
  c-basic-offset:4
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:79
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=79 :
