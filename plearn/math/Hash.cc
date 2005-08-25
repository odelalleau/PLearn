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

#include "Hash.h"

namespace PLearn {
using namespace std;

const unsigned int Hash_UNUSED_TAG   = 0xffffffffu;
const void * Hash_DELETED_SLOT = (void *)0x00000001; // not quite 0


const unsigned int Hash_NOMBRES_MAGIQUES[256]= // un gros tas de nombres magiques 
{
    0x00000000,0x09823b6e,0x130476dc,0x1a864db2,0x2608edb8,0x2f8ad6d6,0x350c9b64,0x3c8ea00a,
    0x4c11db70,0x4593e01e,0x5f15adac,0x569796c2,0x6a1936c8,0x639b0da6,0x791d4014,0x709f7b7a,
    0x9823b6e0,0x91a18d8e,0x8b27c03c,0x82a5fb52,0xbe2b5b58,0xb7a96036,0xad2f2d84,0xa4ad16ea,
    0xd4326d90,0xddb056fe,0xc7361b4c,0xceb42022,0xf23a8028,0xfbb8bb46,0xe13ef6f4,0xe8bccd9a,
    0x39c556ae,0x30476dc0,0x2ac12072,0x23431b1c,0x1fcdbb16,0x164f8078,0x0cc9cdca,0x054bf6a4,
    0x75d48dde,0x7c56b6b0,0x66d0fb02,0x6f52c06c,0x53dc6066,0x5a5e5b08,0x40d816ba,0x495a2dd4,
    0xa1e6e04e,0xa864db20,0xb2e29692,0xbb60adfc,0x87ee0df6,0x8e6c3698,0x94ea7b2a,0x9d684044,
    0xedf73b3e,0xe4750050,0xfef34de2,0xf771768c,0xcbffd686,0xc27dede8,0xd8fba05a,0xd1799b34,
    0x738aad5c,0x7a089632,0x608edb80,0x690ce0ee,0x558240e4,0x5c007b8a,0x46863638,0x4f040d56,
    0x3f9b762c,0x36194d42,0x2c9f00f0,0x251d3b9e,0x19939b94,0x1011a0fa,0x0a97ed48,0x0315d626,
    0xeba91bbc,0xe22b20d2,0xf8ad6d60,0xf12f560e,0xcda1f604,0xc423cd6a,0xdea580d8,0xd727bbb6,
    0xa7b8c0cc,0xae3afba2,0xb4bcb610,0xbd3e8d7e,0x81b02d74,0x8832161a,0x92b45ba8,0x9b3660c6,
    0x4a4ffbf2,0x43cdc09c,0x594b8d2e,0x50c9b640,0x6c47164a,0x65c52d24,0x7f436096,0x76c15bf8,
    0x065e2082,0x0fdc1bec,0x155a565e,0x1cd86d30,0x2056cd3a,0x29d4f654,0x3352bbe6,0x3ad08088,
    0xd26c4d12,0xdbee767c,0xc1683bce,0xc8ea00a0,0xf464a0aa,0xfde69bc4,0xe760d676,0xeee2ed18,
    0x9e7d9662,0x97ffad0c,0x8d79e0be,0x84fbdbd0,0xb8757bda,0xb1f740b4,0xab710d06,0xa2f33668,
    0xe7155ab8,0xee9761d6,0xf4112c64,0xfd93170a,0xc11db700,0xc89f8c6e,0xd219c1dc,0xdb9bfab2,
    0xab0481c8,0xa286baa6,0xb800f714,0xb182cc7a,0x8d0c6c70,0x848e571e,0x9e081aac,0x978a21c2,
    0x7f36ec58,0x76b4d736,0x6c329a84,0x65b0a1ea,0x593e01e0,0x50bc3a8e,0x4a3a773c,0x43b84c52,
    0x33273728,0x3aa50c46,0x202341f4,0x29a17a9a,0x152fda90,0x1cade1fe,0x062bac4c,0x0fa99722,
    0xded00c16,0xd7523778,0xcdd47aca,0xc45641a4,0xf8d8e1ae,0xf15adac0,0xebdc9772,0xe25eac1c,
    0x92c1d766,0x9b43ec08,0x81c5a1ba,0x88479ad4,0xb4c93ade,0xbd4b01b0,0xa7cd4c02,0xae4f776c,
    0x46f3baf6,0x4f718198,0x55f7cc2a,0x5c75f744,0x60fb574e,0x69796c20,0x73ff2192,0x7a7d1afc,
    0x0ae26186,0x03605ae8,0x19e6175a,0x10642c34,0x2cea8c3e,0x2568b750,0x3feefae2,0x366cc18c,
    0x949ff7e4,0x9d1dcc8a,0x879b8138,0x8e19ba56,0xb2971a5c,0xbb152132,0xa1936c80,0xa81157ee,
    0xd88e2c94,0xd10c17fa,0xcb8a5a48,0xc2086126,0xfe86c12c,0xf704fa42,0xed82b7f0,0xe4008c9e,
    0x0cbc4104,0x053e7a6a,0x1fb837d8,0x163a0cb6,0x2ab4acbc,0x233697d2,0x39b0da60,0x3032e10e,
    0x40ad9a74,0x492fa11a,0x53a9eca8,0x5a2bd7c6,0x66a577cc,0x6f274ca2,0x75a10110,0x7c233a7e,
    0xad5aa14a,0xa4d89a24,0xbe5ed796,0xb7dcecf8,0x8b524cf2,0x82d0779c,0x98563a2e,0x91d40140,
    0xe14b7a3a,0xe8c94154,0xf24f0ce6,0xfbcd3788,0xc7439782,0xcec1acec,0xd447e15e,0xddc5da30,
    0x357917aa,0x3cfb2cc4,0x267d6176,0x2fff5a18,0x1371fa12,0x1af3c17c,0x00758cce,0x09f7b7a0,
    0x7968ccda,0x70eaf7b4,0x6a6cba06,0x63ee8168,0x5f602162,0x56e21a0c,0x4c6457be,0x45e66cd0
};



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
