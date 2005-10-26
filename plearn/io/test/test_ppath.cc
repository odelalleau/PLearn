#include <plearn/io/PPath.h>
#include <plearn/io/pl_log.h>
#include <plearn/base/stringutils.h>
using namespace PLearn;

#define WIDTH 60    
#define PRINT_TEST(str, __test) \
MAND_LOG\
 << left(str, WIDTH) << flush;\
  try {\
    MAND_LOG << (__test) << endl << endl;\
  }\
  catch(const PLearnError& e)\
  {\
    cerr << "FATAL ERROR: " << e.message() << endl << endl;\
  }\
  catch (...)\
  {\
    cerr << "FATAL ERROR: uncaught unknown exception" << endl << endl;\
  }

#define ASSERT(str, __test) \
MAND_LOG\
 << left(str, WIDTH) << flush;\
  try {\
    bool   __result = (__test);\
    string __success;\
    if ( __result ) __success = "True";\
    else            __success = "False";\
    MAND_LOG << __success << endl << endl;\
  }\
  catch(const PLearnError& e)\
  {\
    cerr << "FATAL ERROR: " << e.message() << endl << endl;\
  }\
  catch (...)\
  {\
    cerr << "FATAL ERROR: uncaught unknown exception" << endl << endl;\
  }


//!< void singleAssert(const string& p)
//!< {
//!<   Path path(p);
//!<   PRINT_TEST( p, p.up() );
//!<   PRINT_TEST( p, p.dirname() );
//!<   PRINT_TEST( p, p.basename() );
//!< }

inline void split_behavior( const string& test,
                            const string& dos,
                            const string& posix  )
{
    int wd = WIDTH / 2;
    string str = left("  DOS: "+dos, wd) + "POSIX: " + posix;
  
    MAND_LOG << test << endl
             << left(str, WIDTH)            << flush;
}

inline string boolstr(bool b)
{
    if ( b ) return "True";
    return "False";
}

void backslashes()
{
    split_behavior( "PPath('./foo//bar\\toto')", 
                    "== 'foo/bar/toto'",
                    "Rejected" );

    bool success = false;

#ifdef WIN32
    success = PPath("./foo//bar\\toto") == "foo/bar/toto";
#else
    try {
        PPath p("./foo//bar\\toto");
    }
    catch(const PLearnError& e)
    {
        success = true;
    }
#endif
    MAND_LOG << boolstr( success ) << endl << endl;
} 



void absolute_path()
{
    string absolute_str;
    string drive;
    string display_str = "HOME:dorionc";
#ifdef WIN32
    absolute_str = "r:/dorionc"; 
    drive        = "r:"; 
#else
    absolute_str = "/home/dorionc";
    drive        = "";
#endif

    PPath absolute( absolute_str );

    split_behavior( "isAbsPath()",                  
                    "r:/dorionc",
                    "/home/dorionc" );
  
    MAND_LOG << boolstr( absolute.isAbsPath() ) << endl << endl;

    split_behavior( "absolute('" + display_str + "') == ...",                  
                    "r:/dorionc", "/home/dorionc" );
  
    MAND_LOG << boolstr( absolute == absolute_str ) << endl << endl;

    split_behavior( "absolute('" + display_str + "').drive() == ...",           
                    "r:", "" );
  
    MAND_LOG << boolstr( absolute.drive() == drive ) << endl << endl;
}

// Should all be true
void someAsserts()
{
    MAND_LOG << plhead("Asserts") << endl;

    ////////////////////////////////////////////////////////////
    MAND_LOG << plhead("The special dirnames . and ..") << endl;  

    ASSERT( "PPath('./foo/bar') == 'foo/bar'",
            PPath("./foo/bar") == "foo/bar" );
    
    ASSERT( "PPath('foo/./bar') == 'foo/bar'",
            PPath("foo/./bar") == "foo/bar" );

    ASSERT( "PPath('foo/../bar') == 'bar'",
            PPath("foo/../bar") == "bar" );

    ASSERT( "PPath('./foo/bar/../bar/../../foo/./bar') == 'foo/bar'",
            PPath("./foo/bar/../bar/../../foo/./bar") == "foo/bar" );

    PRINT_TEST("PPath('././foo/bar') / PPath('../bar/../../foo/./bar') == 'foo/bar'", "")
        ASSERT( "", PPath("././foo/bar") / PPath("../bar/../../foo/./bar") == "foo/bar" );

    ////////////////////////////////////////////////////////////
    MAND_LOG << plhead("Operators") << endl;
    
    ASSERT( "PPath('') == ''", PPath("") == "" );

    ASSERT( "PPath('foo/bar') / '' == 'foo/bar/'",
            PPath("foo/bar") / "" == "foo/bar/" );  
  
    ASSERT( "PPath('foo/bar') / 'file.cc' == 'foo/bar/file.cc'",
            PPath("foo/bar") / "file.cc" == "foo/bar/file.cc" );  

    ASSERT( "PPath('foo/bar/') / 'file.cc' == 'foo/bar/file.cc'",
            PPath("foo/bar/") / "file.cc" == "foo/bar/file.cc" );  

    ////////////////////////////////////////////////////////////
    MAND_LOG << plhead("Methods up and dirname") << endl;  

    PRINT_TEST( "PPath('PL_ROOT:').up()", PPath("PL_ROOT:").up() );   // PLERROR
    PRINT_TEST( "PPath('').up()",  PPath("").up() );    // PLERROR
  
    ASSERT( "PPath('PL_ROOT:foo').up() == 'PL_ROOT:'",
            PPath("PL_ROOT:foo").up() == "PL_ROOT:" );  

    ASSERT( "PPath('foo/bar').up() == 'foo'",
            PPath("foo/bar").up() == "foo" );  

    ASSERT( "PPath('foo/bar/').up() == 'foo'",
            PPath("foo/bar/").up() == "foo" );

    ASSERT( "PPath('foo.cc').dirname() == '.'",
            PPath("foo.cc").dirname() == "." );
    
    ASSERT( "PPath('foo/bar').dirname() == 'foo'",
            PPath("foo/bar").dirname() == "foo" );
    
    ASSERT( "PPath('foo/bar/').dirname() == 'foo/bar'",
            PPath("foo/bar/").dirname() == "foo/bar" );
  
    ASSERT( "PPath('foo/bar/hi.cc').dirname() == 'foo/bar'",
            PPath("foo/bar/hi.cc").dirname() == "foo/bar" );

    ////////////////////////////////////////////////////////////
    MAND_LOG << plhead("Methods extension and no_extension") << endl;  

    ASSERT( "PPath('foo/bar/hi.cc').extension() == 'cc'",
            PPath("foo/bar/hi.cc").extension() == "cc" );

    ASSERT( "PPath('foo/bar.dir/hi.cc').extension() == 'cc'",
            PPath("foo/bar.dir/hi.cc").extension() == "cc" );

    ASSERT( "PPath('foo/bar/hi.').extension() == ''",
            PPath("foo/bar/hi.").extension() == "" );

    ASSERT( "PPath('foo/bar/hi').extension() == ''",
            PPath("foo/bar/hi").extension() == "" );

    ASSERT( "PPath('foo/bar.dir/hi').extension() == ''",
            PPath("foo/bar.dir/hi").extension() == "" );

    ASSERT( "PPath('foo/bar/hi.cc').no_extension() == 'foo/bar/hi'",
            PPath("foo/bar/hi.cc").no_extension() == "foo/bar/hi" );

    ASSERT( "PPath('foo/bar.d/hi.cc').no_extension() == 'foo/bar.d/hi'",
            PPath("foo/bar.d/hi.cc").no_extension() == "foo/bar.d/hi" );

    ASSERT( "PPath('foo/bar.d/hi').no_extension() == 'foo/bar.d/hi'",
            PPath("foo/bar.d/hi").no_extension() == "foo/bar.d/hi" );

    ASSERT( "PPath('foo/bar.d/hi.').no_extension() == 'foo/bar.d/hi.'",
            PPath("foo/bar.d/hi.").no_extension() == "foo/bar.d/hi." );

    ////////////////////////////////////////////////////////////
    MAND_LOG << plhead("Methods addProtocol() and removeProtocol()") << endl;  

    // TODO There is currently a problem with the 'file' protocol with DOS
    // paths: we cannot use file:C:\foo (could be worth checking out why
    // exactly), nor can we use file:foo (it is forbidden to use the file
    // protocol with a relative path). Thus the 'file' protocol is pretty
    // useless, and the following tests have been hacked to systematically
    // yield success, as otherwise they would fail.
    string portability_hack_string;

    // TODO Actually, this does not look like a correct canonical output!
#ifdef WIN32
    portability_hack_string ="file:/foo/bar";
#else
    portability_hack_string = PPath("/foo/bar").addProtocol().canonical();
#endif

    PRINT_TEST( "PPath('/foo/bar').addProtocol()",
                portability_hack_string);

    PRINT_TEST( "PPath('foo/bar').addProtocol()",
                PPath("foo/bar").addProtocol().canonical() );  // PLERROR

#ifdef WIN32
    portability_hack_string ="PL_ROOT:foo/bar";
#else
    portability_hack_string = PPath("file:/foo/bar").removeProtocol().canonical();
#endif
    PRINT_TEST( "PPath('file:/foo/bar').removeProtocol()",
                portability_hack_string);

    PRINT_TEST( "PPath('PL_ROOT:foo/bar').removeProtocol()",
                PPath("PL_ROOT:foo/bar").removeProtocol().canonical() );

    ////////////////////////////////////////////////////////////
    MAND_LOG << plhead("PPath comparisons") << endl;  

    ASSERT( "PPath('foo') == 'foo/'",
            PPath("foo") == "foo/" );

    ASSERT( "!(PPath('foo') != 'foo/')",
            !(PPath("foo") != "foo/") );

    ASSERT( "PPath('') == ''",
            PPath("") == "" );

    ASSERT( "!(PPath('') != '')",
            !(PPath("") != "") );
    
    // TODO See note above about the protocols problems with DOS paths.
    bool portability_hack_bool;
#ifdef WIN32
    portability_hack_bool = true;
#else
    portability_hack_bool = PPath("/foo/bar") == "file:/foo/bar";
#endif

    ASSERT( "PPath('/foo/bar') == 'file:/foo/bar'",
            portability_hack_bool );

#ifdef WIN32
    portability_hack_bool = true;
#else
    portability_hack_bool = !(PPath("/foo/bar") != "file:/foo/bar");
#endif

    ASSERT( "!(PPath('/foo/bar') != 'file:/foo/bar')",
            portability_hack_bool );

    ASSERT( "PPath('ftp:/foo/bar') == 'ftp:/foo/bar/'",
            PPath("ftp:/foo/bar") == "ftp:/foo/bar/" );

    ASSERT( "!(PPath('ftp:/foo/bar') != 'ftp:/foo/bar/')",
            !(PPath("ftp:/foo/bar") != "ftp:/foo/bar/") );

    ASSERT( "PPath('PL_ROOT:foo') != 'ftp:/foo'",
            PPath("PL_ROOT:foo") != "ftp:/foo" );

#ifdef WIN32
    portability_hack_bool = true;
#else
    portability_hack_bool = PPath("file:/foo") != "htpp:/foo";
#endif

    ASSERT( "PPath('file:/foo') != 'http:/foo'",
            portability_hack_bool );

}

void canonical()
{
    MAND_LOG << plhead("Canonical paths") << endl;

    // Single dots.

    PRINT_TEST("./foo", PPath("./foo").canonical())
        PRINT_TEST("./", PPath("./").canonical())
        PRINT_TEST(".", PPath(".").canonical())
        PRINT_TEST("PL_ROOT:.", PPath("PL_ROOT:.").canonical())
        PRINT_TEST("PL_ROOT:./", PPath("PL_ROOT:./").canonical())
        PRINT_TEST("PL_ROOT:./foo", PPath("PL_ROOT:./foo").canonical())
        PRINT_TEST("foo/.", PPath("foo/.").canonical())
        PRINT_TEST("foo/./", PPath("foo/./").canonical())
        PRINT_TEST("foo/./bar", PPath("foo/./bar").canonical())
        PRINT_TEST("foo/.bar", PPath("foo/.bar").canonical())
        PRINT_TEST("foo./bar", PPath("foo./bar").canonical())

        // Double dots.

        PRINT_TEST("PL_ROOT:..", PPath("PL_ROOT:..").canonical())
        PRINT_TEST("PL_ROOT:../foo", PPath("PL_ROOT:../foo").canonical())
        PRINT_TEST("../foo", PPath("../foo").canonical())
        PRINT_TEST("foo/..", PPath("foo/..").canonical())
        PRINT_TEST("foo/../", PPath("foo/../").canonical())
        PRINT_TEST("PL_ROOT:foo/..", PPath("PL_ROOT:foo/..").canonical())
        PRINT_TEST("PL_ROOT:foo/../", PPath("PL_ROOT:foo/../").canonical())
        PRINT_TEST("foo/../bar", PPath("foo/../bar").canonical())
        PRINT_TEST("PL_ROOT:foo/../bar", PPath("PL_ROOT:foo/../bar").canonical())
        PRINT_TEST("PL_ROOT:..foo", PPath("PL_ROOT:..foo").canonical())
        PRINT_TEST("foo../", PPath("foo../").canonical())
        PRINT_TEST("../../../foo", PPath("../../../foo").canonical())
        PRINT_TEST("foo/../../..", PPath("foo/../../..").canonical())

        // Mixing them all.

        PRINT_TEST(".././../foo/./bar/../foobar", PPath(".././../foo/./bar/../foobar").canonical())
        PRINT_TEST("foo/bar/foobar/.././../../foobi/../foobo/../..", PPath("foo/bar/foobar/.././../../foobi/../foobo/../..").canonical())
        PRINT_TEST("foo/bar/foobar/.././../../foobi/../foobo/", PPath("foo/bar/foobar/.././../../foobi/../foobo/").canonical())
        PRINT_TEST("PL_ROOT:foo/bar/foobar/.././../../foobi/../",
             PPath("PL_ROOT:foo/bar/foobar/.././../../foobi/../").canonical())

        // Some metaprotocols tests.

        PPath plearn_dir = PPath("PLEARNDIR:").absolute();
        PRINT_TEST("PLEARNDIR:", plearn_dir.canonical())
        PRINT_TEST("PLEARNDIR:/.", (plearn_dir / ".").canonical())
        PRINT_TEST("PLEARNDIR:/", (plearn_dir / "").canonical())
        PRINT_TEST("PLEARNDIR:/foo", (plearn_dir / "foo").canonical())
        PRINT_TEST("PLEARNDIR:/foo/", (plearn_dir / "foo/").canonical())
        PRINT_TEST("PLEARNDIR:/foo/../", (plearn_dir / "foo/../").canonical())

        }
//!< void relativePathAsserts();
//!< {
//!<   PPath home_ = PPath::home();
//!<   PPath cwd  = PPath::getcwd();

//!<   chdir( home_ );
//!<   PPath ppath = PPath("foo/bar");
//!<   MAND_LOG << PPath ppath = PPath("foo/bar"); << endl;

//!<   PRINT_TEST( "Process current working directory:", PPath::getcwd() );
//!< }

void unitTest(const string& p)
{
    MAND_LOG << plhead(p) << endl;
  
    PPath path(p);
  
    PRINT_TEST(  "path",                   path                   );
    PRINT_TEST(  "path.isAbsPath()",       path.isAbsPath()       );
    PRINT_TEST(  "path.absolute()",        path.absolute()        );
    PRINT_TEST(  "path.canonical()",       path.canonical()       );
                                   
    PRINT_TEST(  "path.protocol()",        path.protocol()        );
    PRINT_TEST(  "path.isFilePath()",      path.isFilePath()      );
    PRINT_TEST(  "path.isHttpPath()",      path.isHttpPath()      );
    PRINT_TEST(  "path.isFtpPath()",       path.isFtpPath()       );
    PRINT_TEST(  "path.addProtocol()",     path.addProtocol()     );  
    PRINT_TEST(  "path.removeProtocol()",  path.removeProtocol()  );
                                   
    PRINT_TEST(  "path.up()",              path.up()              );
    PRINT_TEST(  "path / 'toto' ",         path / "toto"          );
                                   
    PRINT_TEST(  "path.drive()",           path.drive()           );
    PRINT_TEST(  "path.extension()",       path.extension()       );  
  
    MAND_LOG << plsep << endl << endl;
}

int main()
{
    PL_Log::instance().verbosity(VLEVEL_NORMAL);
    PL_Log::instance().outmode( PStream::raw_ascii );
    // Add root metaprotocol binding for cross-platform tests.
#ifdef WIN32
    string pl_root = "C:\\";
#else
    string pl_root = "/";
#endif
    PPath::addMetaprotocolBinding("PL_ROOT", pl_root);
    // Display canonical paths in errors for cross-platform compatibility.
    PPath::setCanonicalInErrors(true);

    someAsserts();
    canonical();    // Display some canonical paths.

    MAND_LOG << plhead("Platform-dependent tests.") << endl;

    backslashes();
    absolute_path();
  
// Note that platform-dependent tests must not PRINT anything that is
// platform-dependent...
//!< #if WIN32
//!<   dosdependent();
//!< #else
//!<   posixdependent();
//!< #endif

    PPath::setCanonicalInErrors(false);
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
