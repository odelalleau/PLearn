// -*- C++ -*-

// diff.h
//
// Copyright (C) 2005 Olivier Delalleau 
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

// Authors: Olivier Delalleau

/*! \file diff.h */


#ifndef diff_INC
#define diff_INC

#include <plearn/base/Object.h>
#include <map>
#include <string>
#include <plearn/base/OptionBase.h>
#include <plearn/base/tostring.h>
#include <plearn/io/openString.h>
#include <plearn/io/PStream.h>
#include <plearn/math/pl_math.h>    //!< For 'real'.
#include <plearn/math/TVec_decl.h>

using namespace std;

namespace PLearn {

//! Forward declarations.
class Object;
template<class ObjectType, class OptionType> class Option;
//template <class T> class TVec; TODO Use this if possible.
class VMat;
class VMatrix;
class PLearnDiff;
void addDiffPrefix(PLearnDiff* diffs, const string& prefix, int n);
void setSaveDiffs(PLearnDiff* diffs, bool save_diffs,
                                     bool* save_diffs_backup);
int diff(PLearnDiff* diffs,
         const string& refer, const string& other, const string& name);
real get_absolute_tolerance(PLearnDiff* diffs);
real get_relative_tolerance(PLearnDiff* diffs);

// TODO Clean code (more comments, less duplications, maybe less 'hacky' code
// too...).
 
//! Useful function to compare two objects.
int diff(PP<Object> refer, PP<Object> other, PLearnDiff* diffs = 0);

//! Default diff function: compare the two strings.
template<class ObjectType, class OptionType>
int diff(const string& refer, const string& other,
         const Option<ObjectType, OptionType>* opt, PLearnDiff* diffs)
{
    /*
    pout << "Calling basic diff with Option< ObjectType, "
         << opt->optiontype() << " >" << endl;
    */
    PLASSERT( diffs );
    return diff(diffs, refer, other, opt->optionname());
}

//! diff for double.
template<class ObjectType>
int diff(const string& refer, const string& other, const Option<ObjectType, double>* opt, PLearnDiff* diffs)
{
    double x_refer, x_other;
    PStream in = openString(refer, PStream::plearn_ascii);
    in >> x_refer;
    in.flush();
    in = openString(other, PStream::plearn_ascii);
    in >> x_other;
    in.flush();
    if (is_equal(real(x_refer), real(x_other), 1.0,
                 get_absolute_tolerance(diffs), get_relative_tolerance(diffs)))
        return 0;
    else
        return diff(diffs, refer, other, opt->optionname());
}

//! diff for float.
template<class ObjectType>
int diff(const string& refer, const string& other, const Option<ObjectType, float>* opt, PLearnDiff* diffs)
{
    // TODO Avoid code duplication with double.
    float x_refer, x_other;
    PStream in = openString(refer, PStream::plearn_ascii);
    in >> x_refer;
    in.flush();
    in = openString(other, PStream::plearn_ascii);
    in >> x_other;
    in.flush();
    if (is_equal(real(x_refer), real(x_other), 1.0,
                 get_absolute_tolerance(diffs), get_relative_tolerance(diffs)))
        return 0;
    else
        return diff(diffs, refer, other, opt->optionname());
}

//! diff for TVec<>.
template<class ObjectType, class VecElementType>
int diff(const string& refer, const string& other, const Option<ObjectType, TVec<VecElementType> >* opt, PLearnDiff* diffs)
{
    // pout << "Calling diff(..., const Option<ObjectType, TVec<T> > opt, ...)" << endl;
    int n_diffs = 0;
    TVec<VecElementType> refer_vec;
    TVec<VecElementType> other_vec;
    string option = opt->optionname();
    PStream in;
    in = openString(refer, PStream::plearn_ascii);
    in >> refer_vec;
    in = openString(other, PStream::plearn_ascii);
    in >> other_vec;
    in.flush();
    int n = refer_vec.length();
    if (other_vec.length() != n) {
        // pout << "Not same length" << endl;
        // If the two vectors do not have the same size, no need to go further.
        n_diffs += diff(diffs, tostring(n), tostring(other_vec.length()),
                        opt->optionname() + ".length");
    }
    else {
        PP<OptionBase> option_elem = new Option<ObjectType, VecElementType>
            ("", 0, 0, TypeTraits<VecElementType>::name(), "", "", opt->level());//level could be anything here, I guess? -xsm
        string refer_i, other_i;
        // pout << "TVec of " << TypeTraits<VecElementType>::name() << endl;
        for (int i = 0; i < n; i++) {
            option_elem->setOptionName(opt->optionname() + "[" + tostring(i) + "]");
            PStream out = openString(refer_i, PStream::plearn_ascii, "w");
            out << refer_vec[i];
            out.flush();
            out = openString(other_i, PStream::plearn_ascii, "w");
            out << other_vec[i];
            out.flush();
            n_diffs += option_elem->diff(refer_i, other_i, diffs);
        }
    }
    return n_diffs;
}

//! diff for TMat<>.
template<class ObjectType, class MatElementType>
int diff(const string& refer, const string& other, const Option<ObjectType, TMat<MatElementType> >* opt, PLearnDiff* diffs)
{
    // pout << "Calling diff(..., const Option<ObjectType, TMat<T> > opt, ...)" << endl;
    TMat<MatElementType> refer_mat;
    TMat<MatElementType> other_mat;
    string option = opt->optionname();
    PStream in;
    in = openString(refer, PStream::plearn_ascii);
    in >> refer_mat;
    in = openString(other, PStream::plearn_ascii);
    in >> other_mat;
    in.flush();
    int n = refer_mat.length();
    if (other_mat.length() != n)
        // If the two matrices do not have the same length, no need to go further.
        return diff(diffs, tostring(n), tostring(other_mat.length()),
                    opt->optionname() + ".length");
    int w = refer_mat.width();
    if (other_mat.width() != w)
        // If the two matrices do not have the same width, no need to go further.
        return diff(diffs, tostring(w), tostring(other_mat.width()),
                    opt->optionname() + ".width");
    int n_diffs = 0;
    PP<OptionBase> option_elem = new Option<ObjectType, MatElementType>
        ("", 0, 0, TypeTraits<MatElementType>::name(), "", "", opt->level());//level could be anything here, I guess? -xsm
    string refer_ij, other_ij;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < w; j++) {
            option_elem->setOptionName(opt->optionname() + "(" + tostring(i)
                                       + "," + tostring(j) + ")");
            PStream out = openString(refer_ij, PStream::plearn_ascii, "w");
            out << refer_mat(i,j);
            out.flush();
            out = openString(other_ij, PStream::plearn_ascii, "w");
            out << other_mat(i,j);
            out.flush();
            n_diffs += option_elem->diff(refer_ij, other_ij, diffs);
        }
    return n_diffs;
}

//! diff for map.
template<class ObjectType, class MapKeyType, class MapElementType>
int diff(const string& refer, const string& other, const Option<ObjectType,
        map<MapKeyType, MapElementType> >* opt, PLearnDiff* diffs)
{
    // pout << "Calling diff(..., const Option<ObjectType, TVec<T> > opt, ...)" << endl;
    int n_diffs = 0;
    map<MapKeyType, MapElementType> refer_map;
    map<MapKeyType, MapElementType> other_map;
    string option = opt->optionname();
    PStream in;
    in = openString(refer, PStream::plearn_ascii);
    in >> refer_map;
    in = openString(other, PStream::plearn_ascii);
    in >> other_map;
    in.flush();
    PP<OptionBase> option_elem = new Option<ObjectType, MapElementType>
        ("", 0, 0, TypeTraits<MapElementType>::name(), "", "", opt->level());//level could be anything here, I guess? -xsm
    string refer_i, other_i;
    typename map<MapKeyType, MapElementType>::iterator it_refer;
    typename map<MapKeyType, MapElementType>::iterator it_other;
    it_refer = refer_map.begin();
    it_other = other_map.begin();
    TVec<string> missing_in_refer, missing_in_other;
    while (it_refer != refer_map.end()) {
        const MapKeyType& refer_key = it_refer->first;
        option_elem->setOptionName(opt->optionname() + "[ " +
                tostring(refer_key) + " ]");
        if (other_map.find(refer_key) == other_map.end()) {
            // 'refer_map' contains a key 'other_map' does not.
            PStream out =
                openString(refer_i, PStream::plearn_ascii, "w");
            out << refer_key;
            out.flush();
            /*
            refer_i = "<In map>";
            other_i = "<Not in map>";
            n_diffs += option_elem->diff(refer_i, other_i, diffs);
            */
            missing_in_other.append(refer_i);
        } else {
            // Compare the two values.
            const MapElementType& refer_val = it_refer->second;
            const MapElementType& other_val = other_map[refer_key];
            PStream out =
                openString(refer_i, PStream::plearn_ascii, "w");
            out << refer_val;
            out.flush();
            out =
                openString(other_i, PStream::plearn_ascii, "w");
            out << other_val;
            out.flush();
            n_diffs += option_elem->diff(refer_i, other_i, diffs);
        }
        it_refer++;
    }
    // Now look for keys in 'other_map' not present in 'refer_map'.
    while (it_other != other_map.end()) {
        const MapKeyType& other_key = it_other->first;
        option_elem->setOptionName(opt->optionname() + "[ " +
                tostring(other_key) + " ]");
        if (refer_map.find(other_key) == refer_map.end()) {
            PStream out =
                openString(other_i, PStream::plearn_ascii, "w");
            out << other_key;
            out.flush();
            /*
            refer_i = "<Not in map>";
            other_i = "<In map>";
            n_diffs += option_elem->diff(refer_i, other_i, diffs);
            */
            missing_in_refer.append(other_i);
        }
        it_other++;
    }
    /* TODO See how to compare keys properly.
     */
    // Now deal with different keys.
    PP<OptionBase> option_key = new Option<ObjectType, MapKeyType>
        ("", 0, 0, TypeTraits<MapKeyType>::name(), "", "", opt->level());//level could be anything here, I guess? -xsm
    bool save_diffs_backup;
    setSaveDiffs(diffs, false, &save_diffs_backup);
    for (int i = 0; i < missing_in_other.length(); i++) {
        for (int j = 0; j < missing_in_refer.length(); j++) {
            int is_diff =
                option_key->diff(missing_in_other[i], missing_in_refer[j],
                        diffs);
            if (is_diff == 0) {
                // The keys are actually the same (even if not exactly).
                missing_in_refer.remove(j);
                j = missing_in_refer.length(); // No need to go further.
                missing_in_other.remove(i);
                i--;
            }
        }
    }
    setSaveDiffs(diffs, save_diffs_backup, 0);
    for (int i = 0; i < missing_in_other.length(); i++) {
        n_diffs += diff(diffs, "<In map>", "<Not in map>",
                opt->optionname() + "[ " + missing_in_other[i] + " ] ");
    }
    for (int i = 0; i < missing_in_refer.length(); i++) {
        n_diffs += diff(diffs, "<Not in map>", "<In map>",
                opt->optionname() + "[ " + missing_in_refer[i] + " ] ");
    }

    return n_diffs;
}

//! diff for PP<PointedType>.
//! If PointedType does not inherit from Object, use the default 'diff'
//! function that simply compares the two strings.
template<class ObjectType, class PointedType>
int diff(const string& refer, const string& other, const Option<ObjectType, PP<PointedType> >* opt, PLearnDiff* diffs)
{
    // pout << "Calling diff with Option< ObjectType, " << opt->optiontype() << " >" << endl;
    PP<PointedType> refer_pp, other_pp;
    PP<Object> refer_obj, other_obj;
    PStream in = openString(refer, PStream::plearn_ascii);
    in >> refer_pp;
    in = openString(other, PStream::plearn_ascii);
    in >> other_pp;
    refer_obj = dynamic_cast<Object*>((PointedType*) refer_pp);
    if (refer_obj.isNull())
        // This is actually not an object: just compare the two strings.
        return diff(diffs, refer, other, opt->optionname());
    other_obj = dynamic_cast<Object*>((PointedType*) other_pp);
    PLASSERT( other_obj.isNotNull() );
    int n_diffs = diff(refer_obj, other_obj, diffs);
    addDiffPrefix(diffs, opt->optionname() + ".", n_diffs);
    return n_diffs;
}


/*
//! diff for Object.
template<class ObjectType, class OptionType>
int diff(const string& refer, const string& other,
const Option<ObjectType, OptionType>* opt, PLearnDiff* diffs,
// TODO Continue here

const Object* ptr = 0)
{
pout << "Calling diff_Object with Option< ObjectType, " << opt->optiontype() << " >" << endl;
PP<OptionBase> new_opt = new Option<ObjectType, PP<OptionType> >
(opt->optionname(), 0, 0, "", "", "");
return new_opt->diff(refer, other, diffs);
*/

/*
  PP<PointedType> refer_obj; PP<PointedType> other_obj; PStream in =
  openString(refer, PStream::plearn_ascii); in >> refer_obj; in =
  openString(other, PStream::plearn_ascii); in >> other_obj; int n_diffs =
  diff(static_cast<PointedType*>(refer_obj),
  static_cast<PointedType*>(other_obj), diffs); addDiffPrefix(diffs,
  opt->optionname() + ".", n_diffs); return n_diffs;
  return 0;
*/
/*
  }
*/


//! diff for VMat.
#ifndef _MSC_VER
// Currently, this piece of code is problematic with Visual Studio .Net 2003:
// it triggers an error when compiling certain files (e.g. RealMapping.cc)
// that do not depend on VMat.h: even though it looks like in such a case this
// specific diff function should not be compiled, Visual C++ does compile it,
// resulting in an error about the PStream >> VMat operation not being defined.
// As a workaround, this function is thus disabled with Visual Studio: this is
// however not a good solution, since it means diffs performed on a VMat option
// will not behave as expected, and the test PL_diff_VMat will fail.
template<class ObjectType>
int diff(const string& refer, const string& other, const Option<ObjectType, VMat >* opt, PLearnDiff* diffs)
{
    return diff(refer, other,
                (Option<ObjectType, PP<VMatrix> >*) opt, diffs);
}
#endif


//! Add 'prefix' in front of the last 'n' difference names in 'diffs'.
 void addDiffPrefix(const string& prefix, PLearnDiff* diffs, int n);

/*
  template<class ObjectType, class VecElementType>
  int diff(PP<Object> refer, PP<Object> other, const Option<ObjectType, TVec<VecElementType> >* opt, vector<string>& diffs)
  {
  string refer_str, other_str;
  }
*/


/*
//! If 'refer != other, add a new difference with name 'name', reference
//! value 'refer' and other value 'other' to given vector of differences.
//! 'is_diff' is increased by 1 in this case (otherwise it is not changed).
// TODO Update comment.
int diff(const string& refer, const string& other, const string& name,
vector<string>& diffs);
*/

} // end of namespace PLearn

#endif


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
