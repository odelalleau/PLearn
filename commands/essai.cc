#include "TMat.h"

using namespace PLearn;


int nomain()
{
  Vec v(5);
  v << "1 2 3 4 5";
  cout << v << endl;
  PStream mycout(&cout);
  mycout.outmode = PStream::raw_ascii;
  mycout << "raw_ascii" << endl << v << endl;
  mycout.outmode = PStream::pretty_ascii;
  mycout << "pretty_ascii" << endl << v << endl;
  mycout.outmode = PStream::plearn_ascii;
  mycout << "plearn_ascii" << endl << v << endl;
  
  /*
  out.outmode = PStream::plearn_binary;
  out << "plearn_binary" << endl << v << endl;
  out.outmode = PStream::raw_binary;
  out << "raw_binary" << endl << v << endl;
  */

  POFStream out("essai.out");
  out.outmode = PStream::plearn_ascii;
  out << v << endl;
  out.implicit_storage = false;
  out << v << endl;
  out << v.subVec(1,2) << endl;
  out.outmode = PStream::plearn_binary;
  out.implicit_storage = true;
  out << v << endl;
  out.implicit_storage = false;  
  out << v.subVec(1,2) << endl;
  out.flush();
  
  mycout.outmode = PStream::pretty_ascii;
  mycout << "**** AFTER RELOADING: ****" << endl;
  PIFStream in("essai.out");
  Vec v1;
  in >> v1;
  mycout << "v1=" << v1 << endl;
  Vec v2;
  in >> v2;
  mycout << "v2=" << v2 << endl;
  Vec v3;
  in >> v3;
  mycout << "v3=" << v3 << endl;
  v3[0] = 100;
  mycout << "v3=" << v3 << endl;
  mycout << "v2=" << v2 << endl;

  mycout << "--- Reloading plarn_binary stuff ---" << endl;
  Vec v4;
  in >> v4;
  mycout << "v4 = " << v4 << endl;
  Vec v5;
  in >> v5;
  mycout << "v5 = " << v5 << endl;

  return 0;
}



int main()
{
  Mat m(3,2);
  m << 
    "1 2 "
    "3 4 "
    "5 6 ";

  cout << m << endl;
  PStream mycout(&cout);
  mycout.outmode = PStream::raw_ascii;
  mycout << "raw_ascii" << endl << m << endl;
  mycout.outmode = PStream::pretty_ascii;
  mycout << "pretty_ascii" << endl << m << endl;
  mycout.outmode = PStream::plearn_ascii;
  mycout << "plearn_ascii" << endl << m << endl;
  
  /*
  out.outmode = PStream::plearn_binary;
  out << "plearn_binary" << endl << m << endl;
  out.outmode = PStream::raw_binary;
  out << "raw_binary" << endl << m << endl;
  */

  POFStream out("essai.out");
  out.outmode = PStream::plearn_ascii;
  out << m << endl;
  out.implicit_storage = false;
  out << m << endl;
  out << m.subMatRows(1,2) << endl;
  out.outmode = PStream::plearn_binary;
  out.implicit_storage = true;
  out << m << endl;
  out.implicit_storage = false;  
  out << m.subMatRows(1,2).copy() << endl;
  out.flush();
  
  mycout.outmode = PStream::pretty_ascii;
  mycout << "**** AFTER RELOADING: ****" << endl;
  PIFStream in("essai.out");
  Mat m1;
  in >> m1;
  mycout << "m1=" << m1 << endl;
  Mat m2;
  in >> m2;
  mycout << "m2=" << m2 << endl;
  Mat m3;
  in >> m3;
  mycout << "m3=" << m3 << endl;
  m3(0,0) = 100;
  mycout << "m3=" << m3 << endl;
  mycout << "m2=" << m2 << endl;

  mycout << "--- Reloading plarn_binary stuff ---" << endl;
  Mat m4;
  in >> m4;
  mycout << "m4 = " << m4 << endl;
  Mat m5;
  in >> m5;
  mycout << "m5 = " << m5 << endl;

  return 0;
}
