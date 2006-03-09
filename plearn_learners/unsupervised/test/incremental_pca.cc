
#include <plearn/vmat/VMat.h>
#include <plearn/vmat/AutoVMatrix.h>
#include <plearn_learners/unsupervised/PCA.h>

using namespace PLearn;

bool compare( PCA& classical, PCA& incremental,
              VMat data, int start, int end  )
{
    classical.setTrainingSet  ( data.subMatRows(start, end-start), false );
    incremental.setTrainingSet( data.subMatRows(0, end)    , false );
  
    cout << "From " << start << " to " << end << endl;
       
    classical.train();
    incremental.train();

    bool equal = true;
    for ( int i=0; i < classical.eigenvals.length(); i++ )
        if ( not is_equal( classical.eigenvals[i],
                           incremental.eigenvals[i] ) )
        {
            cerr << "classical.eigenvals[" << i << "] = "
                 << classical.eigenvals[i] << endl
                 << "incremental.eigenvals[" << i << "] = "
                 << incremental.eigenvals[i] << endl
                 << endl;
            equal = false;
        }

    if ( equal )
        cout << "OK.\n===\n" << endl;
    else
        cout << "FAILED!!!" << endl;
  
    return equal;
}

int main(int argc, char** argv)
{
    try{
        PCA classical;
        classical.ncomponents         = 3;
        classical.report_progress     = 0;
        classical.normalize_warning   = 0;
        classical.build();
  
        PCA incremental;
        incremental.algo              = "incremental";
        incremental._horizon          = 10;
    
        incremental.ncomponents       = 3;
        incremental.report_progress   = 0;
        incremental.normalize_warning = 0;
        incremental.build();

        VMat data = new AutoVMatrix( "PLEARNDIR:examples/data/test_suite/multi_gaussian_data.amat" );
        compare( classical, incremental, data,  0, 10 );
        compare( classical, incremental, data, 10, 20 );

        for ( int i=11; i <= 20; i++ )
            compare( classical, incremental, data, i, i+10 );
    }
    catch(const PLearnError& e)
    {
        cerr << "FATAL ERROR: " << e.message() << endl;
    }
    catch (...) 
    {
        cerr << "FATAL ERROR: uncaught unknown exception" << endl;
    }
    
    return 0;
}


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
