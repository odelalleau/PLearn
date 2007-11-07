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


/* *******************************************************      
 * $Id$
 * This file is part of the PLearn library.
 ******************************************************* */

/*! DEPRECATED!!!  Use <hash_map> instead.  */

/*! \file PLearn/plearn/math/Hash.h */

#ifndef MODULE_HASH
#define MODULE_HASH

#include <cstdlib>
#include <plearn/base/general.h>
#include <plearn/base/PP.h>

namespace PLearn {
using namespace std;


/*!   KeyType must have 
  a (char *) type cast operator that returns a pointer to its usefull region
  since the key can be any object (therefore, the usefull part of the 
  object does not necessarily starts a offset_0) 
  const size_t byteLength() const  that returns the byte-size of its usefull region,
  new, delete and a copy constructor 
  ==, != 
*/

//!  DataType must have new, delete and copy constructor

//!  Keys are unique

extern const unsigned int Hash_UNUSED_TAG;
extern const void * Hash_DELETED_SLOT;

template <class KeyType, class DataType> 
class HashKeyDataPair
{
public:
    KeyType  *key;
    DataType *data;

    HashKeyDataPair();
    HashKeyDataPair(const KeyType &the_key, const DataType &the_data);
    ~HashKeyDataPair();
};


/////////////////////////////////////////!<  
template <class KeyType, class DataType>
class Hash : public PPointable
{
private:  
    HashKeyDataPair<KeyType,DataType> **table;  //!<  a pointer to a table of <key,data>
    unsigned int tableSize;                     //!<  total number of slots 
    unsigned int elements;                      //!<  number of used slots
    unsigned int pace;                          //!<  used by the secondary hash function
    unsigned int jumps;                         //!<  number of probes in last operation

    unsigned int hashKey(const KeyType &) const; //!<  computes the primary hash function on the string (length=0 says to look for \0 in bytes to determine length)
    void computePace();                         //!<  computes the magic number used by the secondary hash function
    unsigned int pgcd(unsigned int, unsigned int) const;

    unsigned int find(const KeyType &);         //!<  returns either an empty slot or the slot with the key in it
    unsigned int findGap(const KeyType &);      //!<  returns the first empty slot it finds

    //!  the following field are only used in static mode
    bool static_tables_initialized;             //!<  true when the tables are initialized
    KeyType *key_table;
    DataType *data_table;
    HashKeyDataPair<KeyType,DataType> *hash_table;
    unsigned int next_free_slot;                //!<  the next free slot in the 3 tables

public:   
    bool isStatic;                              //!<  static or dynamic memory allocation

    bool add(const KeyType &, const DataType &);  //!<  returns false if table is full
    void addAndResize(const KeyType &, const DataType &);  //!<  add() and resize() if resizeTime()
    bool element(const KeyType &);
    bool del(const KeyType &);

    unsigned int hashAddress(const KeyType &);  //!<  returns Hash_UNUSED_TAG (-1) if not in table

    DataType * operator[](unsigned int addr) const; //!<  data at hashAddress (or 0 if none have been put)
    KeyType * operator()(unsigned int addr) const;  //!<  key at hashAddress (or 0 if none have been put)
    HashKeyDataPair<KeyType,DataType>** getTable() const { return table; }

    unsigned int size() const;                  //!<  returns tableSize;
    unsigned int cardinal() const;              //!<  returns number of elements

    void flush();                               //!<  cleans the table without resizing it

    void initializeTable(unsigned int size);    //!<  initialize the static tables

    //!  diagnostics

    bool full() const;                          //!<  is the table full?
    bool resizeTime() const;                    //!<  is it time to resize the table?
    void resize(int newsize = -1);              //!<  resizes the table to maintain a good sparseness
    //!  (default=-1 quadruples the table size)
    void cleanup();                             //!<  cleanup the table to maintain efficiency
    unsigned int probes() const;                //!<  returns the number of probe generate by last query
    void diagnostics(unsigned int & clusters,   //!<  ...and average cluster size is cardinal()/clusters
                     unsigned int & maxCluster) const;


    Hash(unsigned int size=10000, bool static_allocation=false);
    ~Hash();
private:
    Hash(const Hash& obj) {}  //!<  The copy constructor is not yet defined...
    void operator=(const Hash& obj) {} //!<  not permitted...
};

////////////!<  implementation of HashKeyDataPair ////////////////////////////////////!<  

template <class KeyType, class DataType> 
HashKeyDataPair<KeyType, DataType>::HashKeyDataPair()
    : key(0), data(0)
{
}

template <class KeyType, class DataType> 
HashKeyDataPair<KeyType, DataType>::HashKeyDataPair(const KeyType &the_key, const DataType &the_data)
{
    key = new KeyType(the_key);
    data = new DataType(the_data);
}

template <class KeyType, class DataType> 
HashKeyDataPair<KeyType, DataType>::~HashKeyDataPair()
{
    delete key;
    delete data;
}


////////////!<  implementation of Hash ////////////////////////////////////!<  

///////////////////////////////////!<  

extern const unsigned int Hash_NOMBRES_MAGIQUES[256];

///////////////////////////////////!<  
/*!   
  Computes the remainder (mod AMagicNumber) 
  of the key
*/
template <class KeyType, class DataType> 
unsigned int Hash<KeyType,DataType>::hashKey(const KeyType & key) const
{
    int l = key.byteLength();
    unsigned int hashKey=0u;
    unsigned char *pKey = l>0?((unsigned char *)(char *)key):(unsigned char*)0;
  
    for (int i=0;i<l;i++)
    {
        unsigned char t = (hashKey >> 24);
        hashKey = (hashKey << 8) + pKey[i];
        hashKey ^= Hash_NOMBRES_MAGIQUES[t];
    }

    return hashKey % tableSize;
}

///////////////////////////////////!<  
template <class KeyType, class DataType> 
unsigned int Hash<KeyType,DataType>::pgcd(unsigned int a, unsigned int b) const
{
    while (b)
    {
        unsigned int r=(a % b);
        a=b;
        b=r;
    }
    return a;
}

///////////////////////////////////!<  
/*!   
  The pace is the secondary hash function.
  rather than doing a straight forward linear 
  probing, we do a pseudorandomized probing
*/
template <class KeyType, class DataType> 
void Hash<KeyType,DataType>::computePace()
{
    unsigned int t = (tableSize * 6) / 10;
    while (pgcd(tableSize,t)!=1) t--;
    pace = t;
}

///////////////////////////////////!<  
/*!   
  find will stop either on an empty slot 
  or on a slot that contains the searched 
  string.
*/
template <class KeyType, class DataType> 
unsigned int Hash<KeyType,DataType>::find(const KeyType & key)
{
    unsigned int h = hashKey(key);
/*!     PROBLEME GRAVE ICI: 
  si key->h mais h est occupe, donc key-> jump to h'
  ensuite si table[h] a ete delete
  un find(key) donnera h (qui contient "Hash_DELETED_SLOT") plutot que h'
    
*/
    jumps=0;
    //!  while ( (table[h]>Hash_DELETED_SLOT) &&  //!<  FIX ABOVE PROBLEM
    while ( (table[h]) && (jumps<tableSize) && 
            ((table[h]==Hash_DELETED_SLOT) || (*table[h]->key!=key) ) )
    {
        h = (h+pace) % tableSize;
        jumps++;
    }

    return h;
}

///////////////////////////////////!<  
/*!   
  find will stop either on an empty slot 
  or on a slot that contains the searched 
  string.
*/
template <class KeyType, class DataType> 
unsigned int Hash<KeyType,DataType>::findGap(const KeyType & key)
{
    unsigned int h = hashKey(key);

    jumps=0;
    while ((table[h]>Hash_DELETED_SLOT) && (jumps<tableSize))
    {
        h = (h+pace) % tableSize;
        jumps++;
    }

    return h;
}


///////////////////////////////////!<  
/*!   
  Return False only if the table is 
  full. Use resizeTime() to determine 
  if the table should be resized
*/
template <class KeyType, class DataType> 
bool Hash<KeyType,DataType>::add(const KeyType & key, const DataType & data)
{
    unsigned int h = findGap(key);
    //!  if already element delete it
    if ((table[h]>Hash_DELETED_SLOT) && (*table[h]->key==key))
    {
        if (!isStatic)
            delete table[h];
        table[h]=0;
        elements--;
    }
    if ((table[h]==0) || (table[h]==Hash_DELETED_SLOT))
    {
        if (!isStatic)
            table[h] = new HashKeyDataPair<KeyType,DataType>(key, data);
        else {
            if (static_tables_initialized==false)
                PLERROR("You must initialized the static table before using the add method");
            if (next_free_slot == tableSize)
                return false; //!<  no slot found (bummer!)
            table[h] = &hash_table[next_free_slot];
            table[h]->key  = &key_table[next_free_slot];
            table[h]->data = &data_table[next_free_slot];
            key_table[next_free_slot] = key;
            data_table[next_free_slot] = data;
            next_free_slot++;
        }
        elements++;
        return true;
    }
    else return false; //!<  no slot found (bummer!)
}

///////////////////////////////////!<  
/*!   
  call add() and resize() if resizeTime()
  
*/
template <class KeyType, class DataType> 
void Hash<KeyType,DataType>::addAndResize(const KeyType & key, const DataType & data)
{
    if (resizeTime()) resize();
    add(key,data);
}

///////////////////////////////////!<  
/*!   
  deletes the entry, but leaves a marker 
  so that probes continue to work properly
*/
template <class KeyType, class DataType> 
bool Hash<KeyType,DataType>::del(const KeyType & key)
{ 
    unsigned int h = find(key);

    if (table[h]>Hash_DELETED_SLOT) 
    {
        if (!isStatic)
            delete table[h];
        table[h] = (HashKeyDataPair<KeyType,DataType> *)Hash_DELETED_SLOT;
        elements--;
        return true;
    }
    else return false;
}

///////////////////////////////////!<  
/*!   
  returns true if the string is in the table,
  false otherwise
*/
template <class KeyType, class DataType> 
bool Hash<KeyType,DataType>::element(const KeyType & key)
{
    unsigned int h = find(key);
    return (table[h]>Hash_DELETED_SLOT) && (*table[h]->key==key);
}

///////////////////////////////////!<  
/*!   
  Gives the address in the table of 
  the string (possibly Hash_UNUSED_TAG if 
  there is no such string in the table)
*/
template <class KeyType, class DataType> 
unsigned int Hash<KeyType,DataType>::hashAddress(const KeyType & key)
{
    unsigned int h = find(key);

    if ( (table[h]>Hash_DELETED_SLOT) && (*table[h]->key==key) )
        return h;
    else return Hash_UNUSED_TAG;
}


///////////////////////////////////!<  
//!  
//!  returns the total number of slots
template <class KeyType, class DataType> 
unsigned int Hash<KeyType,DataType>::size() const
{ 
    return tableSize;
}

///////////////////////////////////!<  
//!  
//!  returns the number of used slots.
template <class KeyType, class DataType> 
unsigned int Hash<KeyType,DataType>::cardinal() const
{ 
    return elements;
}


///////////////////////////////////!<  
/*!   
  Tells you if you should resize the 
  table now.
*/
template <class KeyType, class DataType> 
bool Hash<KeyType,DataType>::resizeTime() const
{
    return (elements > (tableSize * 7) / 10);
}

///////////////////////////////////!<  
/*!   
  Tells you if you should resize the 
  table now.
*/
template <class KeyType, class DataType> 
bool Hash<KeyType,DataType>::full() const
{
    return elements == tableSize;
}


///////////////////////////////////!<  
/*!   
  Resizes the table by producing 
  an enlarged copy (not in situ rehashing)
*/
template <class KeyType, class DataType> 
void Hash<KeyType,DataType>::resize(int newsize)
{ 
    //PLWARNING("You are resizing the Hash table in static mode");

    if (newsize<0) newsize=tableSize*4;
    if (newsize<=(int)tableSize) return;
    Hash t(newsize);
    cout << "Resize Hash table to " << newsize << endl;
    for (unsigned int i=0;i<tableSize;i++)
        if (table[i] > Hash_DELETED_SLOT)
            t.add(*table[i]->key,*table[i]->data);

    flush();
    delete[] table;

    table = t.table;
    t.table = 0; //!<  dont deallocate t's table
    tableSize = t.tableSize;
    elements = t.elements;
    pace = t.pace;
    t.table= 0;
}

///////////////////////////////////!<  
/*!   
  Cleans the table after some "deletes"
  
*/
template <class KeyType, class DataType> 
void Hash<KeyType,DataType>::cleanup()
{ 
    Hash t(tableSize);

    for (unsigned int i=0;i<tableSize;i++)
        if (table[i] > Hash_DELETED_SLOT)
            t.add(*table[i]->key,*table[i]->data);

    flush();
    delete[] table;

    table = t.table;
    t.table = 0; //!<  dont deallocate t's table
    tableSize = t.tableSize;
    elements = t.elements;
    pace = t.pace;
    t.table=0;
}


///////////////////////////////////!<  
//!  
//!  Arguments to [] is hashAddress
template <class KeyType, class DataType> 
DataType * Hash<KeyType,DataType>::operator[](unsigned int addr) const
{
    if ((addr<tableSize) && (table[addr]>Hash_DELETED_SLOT))
        return table[addr]->data;
    else return 0;
}


///////////////////////////////////!<  
//!  
//!  Arguments to () is hashAddress
template <class KeyType, class DataType> 
KeyType * Hash<KeyType,DataType>::operator()(unsigned int addr) const
{
    if ((addr<tableSize) && (table[addr]>Hash_DELETED_SLOT))
        return table[addr]->key;
    else return 0;
}



///////////////////////////////////!<  
/*!   
  Erases all strings, but does not 
  destroy the table itself
*/
template <class KeyType, class DataType> 
void Hash<KeyType,DataType>::flush()
{
    if (table)
    {
        for (unsigned int i=0;i<tableSize;i++)
            if (table[i]>Hash_DELETED_SLOT) 
            {
                if (!isStatic)
                    delete table[i];
                table[i]=0;
            }
        elements=0;
    }
}


///////////////////////////////////!<  
//!  
//!  Creates the static table with size entries
template <class KeyType, class DataType> 
void Hash<KeyType, DataType>::initializeTable(unsigned int size)
{
    if (!isStatic)
        PLERROR("The static tables can be initialized only in static mode");

    if (static_tables_initialized==false) {
        key_table = new KeyType[size];
        data_table = new DataType[size];
        hash_table = new HashKeyDataPair<KeyType,DataType>[size];
    }
    else
        PLWARNING("The static tables are already initialized");

    static_tables_initialized = true;
}


///////////////////////////////////!<  
//!  
//!  Creates the table with size entries
template <class KeyType, class DataType> 
Hash<KeyType, DataType>::Hash(unsigned int size, bool static_allocation) 
    : tableSize(size), elements(0), static_tables_initialized(false),
      next_free_slot(0), isStatic(static_allocation)
{
    //!  allocates a table of 0s
    //table = (HashKeyDataPair<KeyType,DataType> **)calloc(size, sizeof(HashKeyDataPair<KeyType,DataType> *));
    assert (size > 0);
    typedef HashKeyDataPair<KeyType,DataType>* hash_key_data_table;
    table = new hash_key_data_table[size];
    for (unsigned int i=0; i<size; i++) table[i]=0;

    computePace();
}


///////////////////////////////////!<  
template <class KeyType, class DataType> 
Hash<KeyType,DataType>::~Hash()
{
    if (table)
    {
        flush();
        delete[] table;
    }
    if (isStatic) {
/*!         before really deleting the hash_table, we set to zero the key and
  data pointers in the elements of this array, to ensure that they
  don't get deleted by the delete[] hash_table (since those pointers
  are not dynamically allocated but rather point into key_table and
  data_table respectively).
*/
        long i, maxi=tableSize;
        for (i=0; i<maxi; ++i) {
            hash_table[i].key = 0;
            hash_table[i].data = 0;
        }
      
        delete[] key_table;
        delete[] data_table;
        delete[] hash_table;
    }
}

///////////////////////////////////!<  
template <class KeyType, class DataType> 
unsigned int Hash<KeyType,DataType>::probes() const
{
    return jumps+1;
}

///////////////////////////////////!<  
#ifndef __max
#define __max(a,b) ((a)>(b)?(a):(b))
#define __maxredefined
#endif

template <class KeyType, class DataType> 
void Hash<KeyType,DataType>::diagnostics(unsigned int & clusters, unsigned int & maxCluster) const
{
    clusters=0;
    maxCluster=0;

    unsigned int d=0;
    while (d<tableSize)
    {
        while ((d<tableSize) && (table[d]==0)) d++;
        if (d<tableSize)
        { 
            clusters++;
            unsigned int c=0;
            while ((d<tableSize) && (table[d])) c++,d++;
            maxCluster=__max(maxCluster,c);
        }
    }
}

/*!   this class is just an "int" and is created only to avoid conflicts
  within Array methods (to build Array<Symbol> because Array<int> does not work)
  It is also useful as a key for Hash tables
*/
struct Symbol
{
    int s;
    Symbol(int symbol=0) : s(symbol) {}
    Symbol(const Symbol& b) : s(b.s) { }
    operator char*() const { return (char*)(&s); }
    // norman: removed useless const
    size_t byteLength() const { return sizeof(int); }
    operator int() { return s; }
    bool operator==(const Symbol& o) const { return o.s==s; }
    bool operator!=(const Symbol& o) const { return o.s!=s; }
    Symbol operator++(int) { return ++s; } //!<  postfix
    Symbol operator++()    { return ++s; } //!<  prefix
};

#ifdef __maxredefined
#undef __maxredefined
#endif

//!  Example of class that can be used as key

class IntPair {
public:
    int i0;
    int i1;
  
    IntPair() { i0=i1=0; }
    IntPair(int j0, int j1) : i0(j0), i1(j1) {}
    IntPair(const IntPair& b) : i0(b.i0), i1(b.i1) { }
    operator char*() const { return (char*)(&i0); }
    int size() const { return 2; }
    // norman: removed useless const
    size_t byteLength() const { return 2*sizeof(int); }
    bool operator!=(const IntPair& b) const { return (i0!=b.i0 || i1!=b.i1); }
    bool operator==(const IntPair& b) const { return (i0==b.i0 && i1==b.i1); }
    int& operator[](int i) { if (i==0) return i0; else return i1; }
};


} // end of namespace PLearn

#endif //!<  MODULE_HASH


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
