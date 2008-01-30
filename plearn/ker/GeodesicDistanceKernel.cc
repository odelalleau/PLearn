// -*- C++ -*-

// GeodesicDistanceKernel.cc
//
// Copyright (C) 2004 Olivier Delalleau 
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

// Authors: Olivier Delalleau

/*! \file GeodesicDistanceKernel.cc */


#include "DistanceKernel.h"
#include <plearn/vmat/FileVMatrix.h>
#include "GeodesicDistanceKernel.h"

namespace PLearn {
using namespace std;

////////////////////////////
// GeodesicDistanceKernel //
////////////////////////////
GeodesicDistanceKernel::GeodesicDistanceKernel() 
    : geodesic_file(""),
      knn(10),
      pow_distance(false),
      shortest_algo("floyd")
{
    distance_kernel = new DistanceKernel(2);
}

GeodesicDistanceKernel::GeodesicDistanceKernel(
        Ker the_distance_kernel, int the_knn,
        const PPath& the_geodesic_file, bool the_pow_distance,
        const string& the_method,
        bool call_build_):
    inherited(true, call_build_),
    geodesic_file(the_geodesic_file),
    knn(the_knn),
    pow_distance(the_pow_distance),
    shortest_algo(the_method)
{
    distance_kernel = the_distance_kernel;
    if (call_build_)
        build_();
}

PLEARN_IMPLEMENT_OBJECT(GeodesicDistanceKernel,
                        "Computes the geodesic distance based on k nearest neighbors.",
                        ""
    );

////////////////////
// declareOptions //
////////////////////
void GeodesicDistanceKernel::declareOptions(OptionList& ol)
{
    // Build options.

    declareOption(ol, "knn", &GeodesicDistanceKernel::knn, OptionBase::buildoption,
                  "The number of nearest neighbors considered.");

    declareOption(ol, "distance_kernel", &GeodesicDistanceKernel::distance_kernel, OptionBase::buildoption,
                  "The kernel giving the distance between two points.");

    declareOption(ol, "pow_distance", &GeodesicDistanceKernel::pow_distance, OptionBase::buildoption,
                  "If set to 1, then it will compute the squared geodesic distance.");

    declareOption(ol, "geodesic_file", &GeodesicDistanceKernel::geodesic_file, OptionBase::buildoption,
                  "If provided, the geodesic distances will be saved in this file in binary format.");

    declareOption(ol, "shortest_algo", &GeodesicDistanceKernel::shortest_algo, OptionBase::buildoption,
                  "The algorithm used to compute the geodesic distances:\n"
                  " - floyd     : Floyd's algorithm\n"
                  " - dijkstra  : Dijkstra's algorithm");

    // Learnt options.

    declareOption(ol, "geo_distances", &GeodesicDistanceKernel::geo_distances, OptionBase::learntoption,
                  "The geodesic distances between training points.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

///////////
// build //
///////////
void GeodesicDistanceKernel::build()
{
    inherited::build();
    build_();
}

////////////
// build_ //
////////////
void GeodesicDistanceKernel::build_()
{
}

/////////////////////////////////////
// computeNearestGeodesicNeighbour //
/////////////////////////////////////
int GeodesicDistanceKernel::computeNearestGeodesicNeighbour(int i, const Mat& distances_xi_x_sorted, real* dist_i) const {
    real min = distances_xi_x_sorted(0,0) + geo_distances->get(i, int(distances_xi_x_sorted(0,1)));
    real dist;
    int indice = 0;
    for (int j = 1; j < knn; j++) {
        dist = distances_xi_x_sorted(j,0) + geo_distances->get(i, int(distances_xi_x_sorted(j,1)));
        if (dist < min) {
            min = dist;
            indice = j;
        }
    }
    if (dist_i)
        *dist_i = min;
    return int(distances_xi_x_sorted(indice,1));
}

/////////////////////////////
// computeShortestDistance //
/////////////////////////////
real GeodesicDistanceKernel::computeShortestDistance(int i, const Mat& distances_xi_x_sorted) const {
    static real result;
    computeNearestGeodesicNeighbour(i, distances_xi_x_sorted, &result);
    return result;
}

//////////////
// evaluate //
//////////////
real GeodesicDistanceKernel::evaluate(const Vec& x1, const Vec& x2) const {
    distance_kernel->computeNearestNeighbors(x1, dist_xi_x_sorted1, knn);
    distance_kernel->computeNearestNeighbors(x2, dist_xi_x_sorted2, knn);
    real min = REAL_MAX;
    real dist;
    for (int j = 0; j < knn; j++) {
        for (int k = 0; k < knn; k++) {
            dist = dist_xi_x_sorted1(j,0) + dist_xi_x_sorted2(k,0)
                + geo_distances->get(int(dist_xi_x_sorted1(j,1)), int(dist_xi_x_sorted2(k,1)));
            if (dist < min) {
                min = dist;
            }
        }
    }
    if (pow_distance) {
        return square(min);
    } else {
        return min;
    }
}

//////////////////
// evaluate_i_j //
//////////////////
real GeodesicDistanceKernel::evaluate_i_j(int i, int j) const {
    if (pow_distance) {
        return square(geo_distances->get(i,j));
    } else {
        return geo_distances->get(i,j);
    }
}

//////////////////
// evaluate_i_x //
//////////////////
real GeodesicDistanceKernel::evaluate_i_x(int i, const Vec& x, real squared_norm_of_x) const {
    return evaluate_i_x_again(i, x, squared_norm_of_x, true);
}

/////////////////////////////////
// evaluate_i_x_from_distances //
/////////////////////////////////
real GeodesicDistanceKernel::evaluate_i_x_from_distances(int i, const Mat& distances_xi_x_sorted) const {
    if (pow_distance) {
        return square(computeShortestDistance(i, distances_xi_x_sorted));
    } else {
        return computeShortestDistance(i, distances_xi_x_sorted);
    }
}

////////////////////////
// evaluate_i_x_again //
////////////////////////
real GeodesicDistanceKernel::evaluate_i_x_again(int i, const Vec& x, real squared_norm_of_x, bool first_time) const {
    if (first_time) {
        distance_kernel->computeNearestNeighbors(x, dist_xi_x_sorted, knn);
    }
    if (pow_distance) {
        return square(computeShortestDistance(i, dist_xi_x_sorted));
    } else {
        return computeShortestDistance(i, dist_xi_x_sorted);
    }
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void GeodesicDistanceKernel::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(dist_xi_x_sorted1, copies);
    deepCopyField(dist_xi_x_sorted2, copies);
    deepCopyField(dist_xi_x_sorted, copies);
    deepCopyField(distance_kernel, copies);
    deepCopyField(geo_distances, copies);
}

////////////////////////////
// setDataForKernelMatrix //
////////////////////////////
void GeodesicDistanceKernel::setDataForKernelMatrix(VMat the_data) {
    inherited::setDataForKernelMatrix(the_data);
    distance_kernel->setDataForKernelMatrix(the_data);
    int n = n_examples;
    // Check whether we have already compute the geodesic distances.
    if (geo_distances && geo_distances->length() == n && geo_distances->width() == n) {
        return;
    }
    // Compute pair distances.
    Mat distances(n,n);
    distance_kernel->computeGramMatrix(distances);
    // Compute knn - nearest neighbors.
    TMat<int> neighborhoods =
        Kernel::computeKNNeighbourMatrixFromDistanceMatrix(
            distances, knn, true, report_progress != 0);
    // Compute geodesic distance by Floyd or Dijkstra's algorithm.
    Mat geodesic(n,n);
    real big_value = REAL_MAX / 3.0; // To make sure no overflow.
    PP<ProgressBar> pb;
    if (report_progress)
        pb = new ProgressBar("Computing geodesic distances", n);
    if (shortest_algo == "floyd") {
        // First initialize the geodesic distances matrix.
        geodesic.fill(big_value);
        int neighbor;
        real d;
        for (int i = 0; i < n; i++) {
            geodesic(i,i) = 0;
            for (int j = 1; j < knn; j++) {
                neighbor = neighborhoods(i,j);
                d = distances(i, neighbor);
                geodesic(i, neighbor) = d;
                geodesic(neighbor, i) = d;
            }
        }
        // And iterate to find geodesic distances.
        real dist;
        for (int k = 0; k < n; k++) {
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < n; j++) {
                    dist = geodesic(i,k) + geodesic(k,j);
                    if (geodesic(i,j) > dist) {
                        geodesic(i,j) = dist;
                    }
                }
            }
            if (report_progress)
                pb->update(k + 1);
        }
    } else if (shortest_algo == "dijkstra") {
        // First build a symmetric neighborhoods matrix
        // (j is a neighbor of i if it was already a neighbor, or if i was a
        // neighbor of j).
        TVec< TVec<int> > sym_neighborhoods(n);
        int neighb, i;
        for (i = 0; i < n; i++) {
            for (int j = 1; j < knn; j++) {
                neighb = neighborhoods(i, j);
                sym_neighborhoods[i].append(neighb);
                sym_neighborhoods[neighb].append(i);
            }
        }
        Vec d;
        TVec<bool> T(n);
        int t, min, j, m, k;
        real dist;
        for (k = 0; k < n; k++) {
            d = geodesic(k);
            d.fill(big_value);
            d[k] = 0;
            T.fill(true);
            for (i = 0; i < n; i++) {
                min = 0;
                while (!T[min])
                    min++;
                for (m = min + 1; m < n; m++) {
                    if (T[m] && d[m] < d[min]) {
                        min = m;
                    }
                }
                for (j = 0; j < sym_neighborhoods[min].length(); j++) {
                    t = sym_neighborhoods[min][j];
                    if (T[t]) {
                        dist = d[min] + distances(min, t);
                        if (d[t] > dist) {
                            d[t] = dist;
                        }
                    }
                }
                T[min] = false;
            }
            if (report_progress)
                pb->update(k+1);
        }
    } else {
        PLERROR("In GeodesicDistanceKernel::setDataForKernelMatrix - Unknown value for 'shortest_algo'");
    }
    // Save the result in geo_distances.
    if (geodesic_file.isEmpty()) {
        geo_distances = VMat(geodesic);
    } else {
        // Use a FileVMatrix to save on disk.
        geo_distances = new FileVMatrix(geodesic_file, n, n);
        geo_distances->putMat(0, 0, geodesic);
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
