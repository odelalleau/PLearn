// -*- C++ -*-

// ManifoldParzen2.cc
//
// Copyright (C) 2003 Pascal Vincent, Julien Keable
// Copyright (C) 2005 University of Montreal
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


/*! \file ManifoldParzen2.cc */
#include "ManifoldParzen2.h"

#include <plearn/math/plapack.h>
#include <plearn/base/general.h>
#include <plearn/math/TMat.h>
#include <plearn/math/TMat_maths.h>
#include <plearn/math/BottomNI.h>
#include <plearn/vmat/VMat_computeNearestNeighbors.h>
#include <plearn/vmat/MemoryVMatrix.h>

namespace PLearn {

PLEARN_IMPLEMENT_OBJECT(ManifoldParzen2,
                        "Manifold Parzen density estimate ",
                        "Parzen Window algorithm, where the covariance matrices of the gaussians are\n"
                        "computed using a knn kernel estimate. Also, only the ncomponents principal\n"
                        "eigen vector and eigen values of the covariance matrices are stored.\n"
    );

/////////////////////
// ManifoldParzen2 //
/////////////////////
ManifoldParzen2::ManifoldParzen2()
    : nneighbors(4),
      ncomponents(1),
      use_last_eigenval(true),
      scale_factor(1),
      learn_mu(false)
{
    type = "general";
    nstages = 1;
}

ManifoldParzen2::ManifoldParzen2(int the_nneighbors, int the_ncomponents, bool use_last_eigenvalue, real the_scale_factor)
    : nneighbors(the_nneighbors),ncomponents(the_ncomponents),use_last_eigenval(true),scale_factor(the_scale_factor)
{
    type = "general";
    nstages = 1;
}

void ManifoldParzen2::build()
{
    inherited::build();
    build_();
}

////////////////////
// declareOptions //
////////////////////
void ManifoldParzen2::declareOptions(OptionList& ol)
{
    declareOption(ol,"nneighbors", &ManifoldParzen2::nneighbors, OptionBase::buildoption,
                  "Number of neighbors for covariance matrix estimation.");

    declareOption(ol,"ncomponents", &ManifoldParzen2::ncomponents, OptionBase::buildoption,
                  "Number of components to store from the PCA.");

    declareOption(ol,"use_last_eigenval", &ManifoldParzen2::use_last_eigenval, OptionBase::buildoption,
                  "Indication that the last eigenvalue should be used for the remaining directions' variance.");

    declareOption(ol,"learn_mu", &ManifoldParzen2::learn_mu, OptionBase::buildoption,
                  "Indication that the difference vector between the training points and the gaussians should be learned.\n"
                  "By default, the gaussians are centered at the training points.");

    declareOption(ol,"global_lambda0", &ManifoldParzen2::global_lambda0, OptionBase::buildoption,
                  "If use_last_eigenvalue is false, used value for the minimum variance in all directions");

    declareOption(ol,"scale_factor", &ManifoldParzen2::scale_factor, OptionBase::buildoption,
                  "Scale factor");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);

    // Redeclare some parent's options.

    redeclareOption(ol,"L", &ManifoldParzen2::L, OptionBase::learntoption,
                    "Automatically set (to train_set->length()).");

    redeclareOption(ol,"n_eigen", &ManifoldParzen2::n_eigen, OptionBase::learntoption,
                    "Automatically set during training.");

    redeclareOption(ol,"type", &ManifoldParzen2::type, OptionBase::nosave,
                    "Automatically set (to 'general').");

    redeclareOption(ol,"alpha_min", &ManifoldParzen2::alpha_min, OptionBase::nosave,
                    "Not used.");

    redeclareOption(ol,"kmeans_iterations", &ManifoldParzen2::kmeans_iterations, OptionBase::nosave,
                    "Not used.");

}

void ManifoldParzen2::build_()
{}

void ManifoldParzen2::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields
    // ### that you wish to be deepCopied rather than
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    //PLERROR("ManifoldParzen2::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

// This is an efficient version of the most basic nearest neighbor search, using a Mat and euclidean distance
void computeNearestNeighbors(Mat dataset, Vec x, Mat& neighbors, int ignore_row=-1)
{
    int K = neighbors.length(); // how many neighbors do we want?
    BottomNI<real> neighbs(K);
    for(int i=0; i<dataset.length(); i++)
        if(i!=ignore_row)
            neighbs.update(powdistance(dataset(i),x), i);
    neighbs.sort();
    TVec< pair<real,int> > indices = neighbs.getBottomN();
    int nonzero=0;
    for(int k=0; k<K; k++)
    {
        if(indices[k].first>0)
            nonzero++;
        neighbors(k) << dataset(indices[k].second);
    }
    if(nonzero==0)
        PLERROR("All neighbors had 0 distance. Use more neighbors. (There were %i other patterns with same values)",neighbs.nZeros());
}

// computes the first components.length() principal components of the rows of datset.
// Result will be put in the rows of components (which is expected to have the appropriate size).
// Computed components do not have a norm of 1, but rather a norm corresponding to the eigenvalues
// (indicating how important the component is...)
void computePrincipalComponents(Mat dataset, Vec& eig_values, Mat& eig_vectors, real global_lambda0)
{
#ifdef BOUNDCHECK
    if(eig_vectors.width()!=dataset.width())
        PLERROR("In computePrincipalComponents eig_vectors and dataset must have same width");
    if(eig_values.length() != eig_vectors.length())
        PLERROR("In computePrincipalComponents eig_values vec and eig_vectors mat must have same length");
#endif

    static Mat covar;
    int ncomp = eig_values.length(); // number of components we want
    covar.resize(dataset.width(), dataset.width());
    transposeProduct(covar, dataset,dataset);
    eigenVecOfSymmMat(covar, ncomp,  eig_values, eig_vectors);
    for (int i=0;i<eig_values.length();i++)
    {
        if (eig_values[i]<0)
            eig_values[i] = 0;
        eig_values[i] = eig_values[i]/dataset.length() + global_lambda0;
    }
}

void computeLocalPrincipalComponents(Mat& dataset, int which_pattern, Mat& delta_neighbors, Vec& eig_values, Mat& eig_vectors, Vec& mean, bool learn_mu=false,real global_lambda0=0)
{
    Vec center_ = dataset(which_pattern);
    if (center_.hasMissing())
        PLERROR("dataset row %d has missing values!", which_pattern);
    computeNearestNeighbors(dataset, center_, delta_neighbors, which_pattern);
    if(learn_mu)
    {
        mean.resize(delta_neighbors.width());  // Hugo: the mean should be the current point...
        columnMean(delta_neighbors, mean);
    }
    delta_neighbors -= mean;
    computePrincipalComponents(delta_neighbors, eig_values, eig_vectors,global_lambda0);
}

void ManifoldParzen2::train()
{
    Mat trainset(train_set);
    int l = train_set.length();
    int w = train_set.width();
    eigenval_copy.resize(ncomponents+1);
    row.resize(w);

    L = l;
    D = ncomponents;
    n_eigen = ncomponents;
    GaussMix::build(); // TODO Still needed?
    // resizeStuffBeforeTraining();
//  setMixtureTypeGeneral(l, ncomponents, w); // TODO Remove this line when it works.

    // storage for neighbors
    Mat delta_neighbors(nneighbors, w);
    Vec eigvals(ncomponents+1);
    Mat components_eigenvecs(ncomponents+1,w);
    for(int i=0; i<l; i++)
    {
        if(i%100==0)
            cerr << "[SEQUENTIAL TRAIN: processing pattern #" << i << "/" << l << "]\n";

        // center is sample
        center(i) << trainset(i);

        Vec center_(center.width());
        center_ << trainset(i);
        if(use_last_eigenval)
            computeLocalPrincipalComponents(trainset, i, delta_neighbors,
                    eigvals, components_eigenvecs, center_,learn_mu);
        else
            computeLocalPrincipalComponents(trainset, i, delta_neighbors,
                    eigvals, components_eigenvecs, center_,learn_mu,global_lambda0);

        eigvals *= scale_factor;

//    cout<<delta_neighbors<<endl;

        real d=0;
        for(int k=0;k<delta_neighbors.length();k++)
            d+=dist(delta_neighbors(k),Vec(D,0.0),2);
        d/=delta_neighbors.length();

        // find out eigenvalue (a.k.a lambda0) that will be used for all D-K directions
        real lambda0;
        if(use_last_eigenval)
        {
            // take last (smallest) eigenvalue as a variance in the non-principal directions
            // (but if it is 0 because of linear dependencies in the data, take the
            // last, i.e. smallest, non-zero eigenvalue).
            int last=ncomponents;
            lambda0 = eigvals[last];
            while (fast_exact_is_equal(lambda0, 0) && last>0)
                lambda0 = eigvals[--last];
            // the sigma-square for all remaining dimensions
            if (fast_exact_is_equal(lambda0, 0))
                PLERROR("All (%i) principal components have zero variance!?",eigvals.length());
        }
        else lambda0 = global_lambda0;

        alpha[i] = 1.0 / l;
        n_eigen = eigvals.length() - 1;
        //GaussMix::build();
        //resizeStuffBeforeTraining();
        if(learn_mu)
            center(i) << center_;   // Hugo: the mean should be current point...
        eigenvalues(i) << eigvals;
        // eigenvalues(i, n_eigen_computed - 1) = lambda0; TODO Put back!
        eigenvectors[i] << components_eigenvecs;
//    setGaussianGeneral(i, 1.0/l, center, eigvals.subVec(0,eigvals.length()-1), components_eigenvecs.subMatRows(0,eigvals.length()-1), lambda0);
    }
    stage = 1;
    // precomputeStuff(); TODO Put back?
    build();
}

int ManifoldParzen2::find_nearest_neighbor(VMat data, Vec x) const
{
    int ret = -1;
    real distance = MISSING_VALUE;
    real temp;
    for(int i=0; i<data->length(); i++)
    {
        data->getRow(i,row);
        temp = dist(row,x,2);
        if(is_missing(distance) || temp<distance)
        {
            distance = temp;
            ret = i;
        }
    }
    return ret;
}

real ManifoldParzen2::evaluate(Vec x1,Vec x2,real scale)
{
    real ret;
    int i = find_nearest_neighbor(train_set,x2);

    if(fast_exact_is_equal(scale, 1))
        ret = computeLogLikelihood(x1,i);
    else
    {
        eigenval_copy << eigenvalues(i);
        eigenvalues(i) *= scale;

        // Maybe sigma_min should be adjusted!

        ret = computeLogLikelihood(x1,i);

        eigenvalues(i) << eigenval_copy;
    }
    return exp(ret);

    /*
      row = x1 - mu(i);
      ret = 0.5 * scale * pownorm(row)/eigenvalues(i,n_eigen_computed - 1);
      for (int k = 0; k < n_eigen_computed - 1; k++) {
      ret += 0.5 * scale * (1.0 / eigenvalues(i,k) - 1.0/eigenvalues(i,n_eigen_computed-1)) * square(dot(eigenvectors[i](k), row));
      }
      return ret;
    */
}

real ManifoldParzen2::evaluate_i_j(int i,int j,real scale)
{
    real ret;

    if(fast_exact_is_equal(scale, 1))
        ret = computeLogLikelihood(center(i),j);
    else
    {
        eigenval_copy << eigenvalues(j);
        eigenvalues(j) *= scale;

        // Maybe sigma_min should be adjusted!

        ret = computeLogLikelihood(center(i),j);

        eigenvalues(j) << eigenval_copy;
    }
    return exp(ret);

    /*
      row = mu(i) - mu(j);
      ret = scale * pownorm(row)/eigenvalues(j,n_eigen_computed - 1);
      for (int k = 0; k < n_eigen_computed - 1; k++) {
      ret += scale * (1.0 / eigenvalues(j,k) - 1.0/eigenvalues(j,n_eigen_computed-1)) * square(dot(eigenvectors[j](k), row));
      }
      return ret;
    */
}



///////////////////
// computeOutput //
///////////////////
void ManifoldParzen2::computeOutput(const Vec& input, Vec& output) const
{
    switch(outputs_def[0])
    {
    case 'r':
    {
        int i, last_i=-1;
        int nstep = 100000;
        real step = 0.001;
        int save_every = 100;
        string fsave = "";
        string musave = "";
        VMat temp;
        t_row.resize(input.length());
        row.resize(input.length());
        t_row << input;
        mu_temp.resize(center.length(),center.width());
        temp_eigv.resize(input.length());
        mu_temp << center;
        for(int s=0; s<nstep;s++)
        {
            i = find_nearest_neighbor(new MemoryVMatrix(mu_temp),t_row);

            if(s % save_every == 0)
            {
                fsave = "mp_walk_" + tostring(s) + ".amat";
                temp = new MemoryVMatrix(t_row.toMat(1,t_row.length()));
                temp->saveAMAT(fsave,false,true);

                musave = "mp_mu_" + tostring(s) + ".amat";
                temp = new MemoryVMatrix(mu_temp(i).toMat(1,mu_temp(i).length()));
                temp->saveAMAT(musave,false,true);

            }
            temp_eigv << eigenvectors[i](0);
            real sign = (last_i == -1 || dot(eigenvectors[i](0),eigenvectors[last_i](0)) >= 0 ? 1 : -1);
            t_row += step*sign*temp_eigv ;
            last_i = i;
        }
        output << t_row;
        break;
    }
    default:
        inherited::computeOutput(input,output);
    }
}

////////////////
// outputsize //
////////////////
int ManifoldParzen2::outputsize() const
{
    switch(outputs_def[0])
    {
    case 'r':
        return eigenvectors[0].width();
    default:
        return inherited::outputsize();
    }
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
