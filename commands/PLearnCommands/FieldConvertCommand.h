#ifndef FieldConvertCommand_INC
#define FieldConvertCommand_INC

#include "PLearnCommand.h"
#include "PLearnCommandRegistry.h"

namespace PLearn <%
using namespace std;

class FieldConvertCommand: public PLearnCommand
{
public:
  FieldConvertCommand():
    PLearnCommand("FieldConvert",

                  "Reads a dataset and generates a .vmat file based on the data, but optimized for training.\n",

                  "The nature of each field of the original dataset is automatically detected, and determines the approriate treatment.\n"\
                  "The possible field types with the corresponding treatment can be one of :\n"\
                  "continuous      - quantitative data (data is real): the field is replaced by the normalized data (minus means, divided by stddev)\n"\
                  "discrete_uncorr - discrete integers (qualitative data, e.g : postal codes, categories) not corr. with target: the field is replaced by a group of fields in a one-hot fashion.\n"\
                  "discrete_corr   - discrete integers, correlated with target : both the normalised and the onehot versions of the field are used in the new dataset\n"\
                  "constant        - constant data : the field is skipped (it is not present in the new dataset)\n"\
                  "skip            - unrelevant data : the field is skipped (it is not present in the new dataset)\n"\
/*                  "1 - quantitative data (data is real): the field is replaced by the normalized data (minus means, divided by stddev)\n"\
                  "2 - discrete integers (qualitative data, e.g : postal codes, categories) not corr. with target: the field is replaced by a group of fields in a one-hot fashion.\n"\
                  "3 - discrete integers, correlated with target : both the normalised and the onehot versions of the field are used in the new dataset\n"\
                  "4 - constant data : the field is skipped (it is not present in the new dataset)\n"\
                  "5 - unrelevant data : the field is skipped (it is not present in the new dataset)\n"\ */
                  "\n"\
                  "When there are ambiguities, messages are displayed for the problematic field(s) and they are skipped. The user must use a 'force' file,\n"\
                  "to explicitely force the types of the ambiguous field(s). The file is made of lines of the following 2 possible formats:\n"\
                  "FIELDNAME=type\n"\
                  "fieldNumberA-fieldNumberB=type   [e.g : 200-204=constant, to force a range]\n"\
                  "\n"\
                  "Note that all types but skip, if the field contains missing values, an additionnal 'missing-bit' field is added and is '1' only for missing values.\n"\
                  "(A constant field with NaNs is represented only through a missing bit field in the new dataset)\n"\
                  "the difference between types constant and skip is only cosmetic: constant means the field is constant, while skip means either there are too many missing values or it has been forced to skip.\n"\
                  "A report file is generated and contains the information about the processing for each field.\n"\
                  "Target index of source needs to be specified (ie. to perform corelation test). It can be any field of the "\
                  "source dataset, but will be the last field of the new dataset.*** We assume target is never missing *** \n\n"\
                  "usage : FieldConvert *source=[source dataset] *destination=[new dataset with vmat extension] *target=[field index of target]\n"\
                  "force=[force file] report=[report file] fraction=[if number of unique values is > than 'fraction' * NonMISSING -> the field is continuous. Default=.3]\n"\
                  "max_pvalue=[maximum pvalue to assume correlation with target, default=0.025] frac_missing_to_skip=[if MISSING >= 'frac_missing_to_skip * number of samples then this field is skipped, default=0.9]\n"\
                  "\nfields with asterix * are not optional\n"
                  ) 
  {}
  // TODO Check this affirmation :
  //"(A constant field with NaNs is represented only through a missing bit field in the new dataset)\n"
                    
  //! The available types for a field.
  enum FieldType {
    continuous,
    discrete_uncorr,
    discrete_corr,
    constant,
    skip,
    weird,  //< Undocumented type under investigation.
    unknown //< Default value before the decision on the field type is made.
  };
  
  virtual void run(const vector<string>& args);

protected:

  // Convert a string into our enum type.
  FieldType stringToFieldType(string s);

  static PLearnCommandRegistry reg_;

  float UNIQUE_NMISSING_FRACTION_TO_ASSUME_CONTINUOUS;
  float PVALUE_THRESHOLD;
  float FRAC_MISSING_TO_SKIP;
  string source_fn, desti_fn,force_fn,report_fn;
  bool onehot_with_correl;
  int target;
  FieldType type;
};

  
%> // end of namespace PLearn

#endif

