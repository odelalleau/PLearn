#include "Template.h"

namespace PLearn {


Template::Template(){
}

PLEARN_IMPLEMENT_OBJECT(Template,
    "A Molecule",
    ""
);

void Template::declareOptions(OptionList& ol)
{
  declareOption(ol, "dev", &Template::dev, OptionBase::buildoption,
                "Chemical Properties");

  // Now call the parent class' declareOptions
  inherited::declareOptions(ol);
}

void Template::build_()
{}

void Template::build()
{
  inherited::build();
  build_();
}

void Template::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);
  deepCopyField(dev, copies);
  
}

}

