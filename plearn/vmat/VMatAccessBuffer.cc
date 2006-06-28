// -*- C++ -*-

// VMatAccessBuffer.cc
//
// Copyright (C) 2006 Christian Dorion
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

// Authors: Christian Dorion

/*! \file VMatAccessBuffer.cc */


#include "VMatAccessBuffer.h"

namespace PLearn {
using namespace std;

VMatAccessBuffer::
VMatAccessBuffer(VMat source, int max_size)
    : m_source( source ),
      m_current_row( -1 ),
      m_max_size( max_size )
{
    m_row_buffer.resize(m_source.width());
}

void
VMatAccessBuffer::
getRow(int row, const Vec& rowbuf)
{    
    int last_row = m_current_row + m_cached_rows.size() - 1;        

    bool much_below = false;
    if ( row < m_current_row )
    {
        int new_size = last_row - row + 1;
        much_below = ( new_size > m_max_size );
        if ( !much_below )
            while ( m_current_row > row ) {
                m_source->getRow(--m_current_row, m_row_buffer);
                m_cached_rows.push_front(m_row_buffer.copy());
            }
    }

    if ( much_below || row > last_row )
    {
        m_source->getRow(row, m_row_buffer);
        
        m_current_row = row;
        m_cached_rows.clear();
        m_cached_rows.push_back(m_row_buffer.copy());        
    }
    
    else
    {
        for ( ; m_current_row < row; ++m_current_row )
            m_cached_rows.pop_front();        
        m_row_buffer << m_cached_rows[0];
    }
    
    // Finally    
    rowbuf << m_row_buffer;
    assert( m_current_row == row );
    assert( int(m_cached_rows.size()) <= m_max_size );
}

void
VMatAccessBuffer::
lookAhead(int row, const Vec& rowbuf)
{
    assert( row > m_current_row );
    int last_row = m_current_row + m_cached_rows.size() - 1;

    if ( row <= last_row )
        m_row_buffer << m_row_buffer[row-m_current_row];
    else
        for ( ; last_row < row; ++last_row )
        {
            m_source->getRow(row, m_row_buffer);
            m_cached_rows.push_back(m_row_buffer);
        }
    
    // Finally    
    rowbuf << m_row_buffer;
    assert( int(m_cached_rows.size()) <= m_max_size );
}

//! Deep copying
VMatAccessBuffer* VMatAccessBuffer::deepCopy(CopiesMap& copies) const
{
    CopiesMap::iterator it = copies.find(this);
    if(it!=copies.end())  //!<  a copy already exists, so return it
        return (VMatAccessBuffer*) it->second;
  
    //! Otherwise call the copy constructor to obtain a copy
    VMatAccessBuffer* deep_copy = new VMatAccessBuffer(*this);

    deepCopyField(this->m_source,      copies);
    deepCopyField(this->m_row_buffer,  copies);
    deepCopyField(this->m_cached_rows, copies);
    
    //!  Put the copy in the map
    copies[this] = deep_copy;

    //!  return the completed deep_copy
    return deep_copy;
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
