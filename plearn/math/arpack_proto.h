// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
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
 
#ifndef arpack_proto_INC
#define arpack_proto_INC

#define FORTRAN_Integer int

namespace PLearn {
using namespace std;


extern "C"
{  
    int dnaupd_(FORTRAN_Integer *, const char *, FORTRAN_Integer *, const char *, 
                FORTRAN_Integer *, double *, double *, FORTRAN_Integer *, double *, 
                FORTRAN_Integer *, FORTRAN_Integer *, FORTRAN_Integer *, double *, double *, 
                FORTRAN_Integer *, FORTRAN_Integer *, short, short);
  
    int dneupd_(FORTRAN_Integer *, const char *, FORTRAN_Integer *, double *, double *, 
                double *, FORTRAN_Integer *, double *, double *, double *, 
                const char *, FORTRAN_Integer *, const char *, FORTRAN_Integer *, double *, 
                double *, 
                FORTRAN_Integer *, double *, FORTRAN_Integer *, FORTRAN_Integer *, FORTRAN_Integer *, 
                double *, double *, FORTRAN_Integer *, FORTRAN_Integer *, short, short,
                short);
   
    int snaupd_(FORTRAN_Integer *, const char *, FORTRAN_Integer *, const char *, 
                FORTRAN_Integer *, float *, float *, FORTRAN_Integer *, float *, 
                FORTRAN_Integer *, FORTRAN_Integer *, FORTRAN_Integer *, float *, float *, 
                FORTRAN_Integer *, FORTRAN_Integer *, short, short);
  
    int sneupd_(FORTRAN_Integer *, const char *, FORTRAN_Integer *, float *, float *, 
                float *, FORTRAN_Integer *, float *, float *, float *, 
                const char *, FORTRAN_Integer *, const char *, FORTRAN_Integer *, float *, 
                float *, 
                FORTRAN_Integer *, float *, FORTRAN_Integer *, FORTRAN_Integer *, FORTRAN_Integer *, 
                float *, float *, FORTRAN_Integer *, FORTRAN_Integer *, short, short,
                short);

    int dsaupd_(FORTRAN_Integer *, const char *, FORTRAN_Integer *, const char *, 
                FORTRAN_Integer *, double *, double *, FORTRAN_Integer *, double *, 
                FORTRAN_Integer *, FORTRAN_Integer *, FORTRAN_Integer *, double *, double *, 
                FORTRAN_Integer *, FORTRAN_Integer *, short, short);
  
    int dseupd_(FORTRAN_Integer *, const char *, FORTRAN_Integer *, double *, 
                double *, FORTRAN_Integer *, double *, 
                const char *, FORTRAN_Integer *, const char *, FORTRAN_Integer *, double *, 
                double *, 
                FORTRAN_Integer *, double *, FORTRAN_Integer *, FORTRAN_Integer *, FORTRAN_Integer *, 
                double *, double *, FORTRAN_Integer *, FORTRAN_Integer *, short, short,
                short);
   
    int ssaupd_(FORTRAN_Integer *, const char *, FORTRAN_Integer *, const char *, 
                FORTRAN_Integer *, float *, float *, FORTRAN_Integer *, float *, 
                FORTRAN_Integer *, FORTRAN_Integer *, FORTRAN_Integer *, float *, float *, 
                FORTRAN_Integer *, FORTRAN_Integer *, short, short);
  
    int sseupd_(FORTRAN_Integer *, const char *, FORTRAN_Integer *, float *, 
                float *, FORTRAN_Integer *, float *, 
                const char *, FORTRAN_Integer *, const char *, FORTRAN_Integer *, float *, 
                float *, 
                FORTRAN_Integer *, float *, FORTRAN_Integer *, FORTRAN_Integer *, FORTRAN_Integer *, 
                float *, float *, FORTRAN_Integer *, FORTRAN_Integer *, short, short,
                short);
}

} // end of namespace PLearn


#endif /* arpack_proto_INC */


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
