/* -*- Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2012 University of California, Los Angeles
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Zhenkai Zhu <zhenkai@cs.ucla.edu>
 *         Chaoyi Bian <bcy@pku.edu.cn>
 *	   Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 */

#include "sync-digest.h"
#include <string.h>

#include <boost/assert.hpp>
#include <boost/throw_exception.hpp>
typedef boost::error_info<struct tag_errmsg, std::string> errmsg_info_str; 
typedef boost::error_info<struct tag_errmsg, int> errmsg_info_int; 

// for printing, may be disabled in optimized build

// #ifdef DIGEST_BASE64
// #include <boost/archive/iterators/base64_from_binary.hpp>
// #include <boost/archive/iterators/binary_from_base64.hpp>
// #endif

#include <boost/archive/iterators/transform_width.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/archive/iterators/dataflow_exception.hpp>

using namespace boost;
using namespace boost::archive::iterators;
using namespace std;

// Other options: VP_md2, EVP_md5, EVP_sha, EVP_sha1, EVP_sha256, EVP_dss, EVP_dss1, EVP_mdc2, EVP_ripemd160
#define HASH_FUNCTION EVP_sha256


// #ifndef DIGEST_BASE64

template<class CharType>
struct hex_from_4_bit
{
  typedef CharType result_type;
  CharType operator () (CharType ch) const
  {
    const char *lookup_table = "0123456789abcdef";
    // cout << "New character: " << (int) ch << " (" << (char) ch << ")" << "\n";
    BOOST_ASSERT (ch < 16);
    return lookup_table[static_cast<size_t>(ch)];
  }
};

typedef transform_iterator<hex_from_4_bit<string::const_iterator::value_type>,
                           transform_width<string::const_iterator, 4, 8, string::const_iterator::value_type> > string_from_binary;


template<class CharType>
struct hex_to_4_bit
{
  typedef CharType result_type;
  CharType operator () (CharType ch) const
  {
    const signed char lookup_table [] = {
      -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
      -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
      -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
      0, 1, 2, 3, 4, 5, 6, 7, 8, 9,-1,-1,-1,-1,-1,-1,
      -1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,
      -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
      -1,10,11,12,13,14,15,-1,-1,-1,-1,-1,-1,-1,-1,-1,
      -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1
    };

    // cout << "New character: " << hex << (int) ch << " (" << (char) ch << ")" << "\n";
    signed char value = -1;
    if ((unsigned)ch < 128)
      value = lookup_table [(unsigned)ch];
    if (value == -1)
      BOOST_THROW_EXCEPTION (Sync::Error::DigestCalculationError () << errmsg_info_int ((int)ch));
    
    return value;
  }
};

typedef transform_width<transform_iterator<hex_to_4_bit<string::const_iterator::value_type>, string::const_iterator>, 8, 4> string_to_binary;

// #else

// typedef base64_from_binary<transform_width<string::const_iterator, 6, 8> > string_from_binary;
// typedef binary_from_base64<transform_width<string::const_iterator, 8, 6> > string_to_binary;

// #endif

namespace Sync {

Digest::Digest ()
  : m_buffer (0)
  , m_hashLength (0)
{
  m_context = EVP_MD_CTX_create ();

  reset ();
}

Digest::~Digest ()
{
  if (m_buffer != 0)
    delete [] m_buffer;

  EVP_MD_CTX_destroy (m_context);
}

bool
Digest::empty () const
{
  return m_buffer == 0;
}

bool
Digest::isZero () const
{
  if (m_buffer == 0)
    BOOST_THROW_EXCEPTION (Error::DigestCalculationError ()
                           << errmsg_info_str ("Digest has not been yet finalized"));

  return (m_hashLength == 1 && m_buffer[0] == 0);
}


void
Digest::reset ()
{
  if (m_buffer != 0)
    {
      delete [] m_buffer;
      m_buffer = 0;
    }

  int ok = EVP_DigestInit_ex (m_context, HASH_FUNCTION (), 0);
  if (!ok)
    BOOST_THROW_EXCEPTION (Error::DigestCalculationError ()
                           << errmsg_info_str ("EVP_DigestInit_ex returned error")
                           << errmsg_info_int (ok));
}


void
Digest::finalize ()
{
  if (m_buffer != 0) return;

  m_buffer = new uint8_t [EVP_MAX_MD_SIZE];

  int ok = EVP_DigestFinal_ex (m_context,
			       m_buffer, &m_hashLength);
  if (!ok)
    BOOST_THROW_EXCEPTION (Error::DigestCalculationError ()
                           << errmsg_info_str ("EVP_DigestFinal_ex returned error")
                           << errmsg_info_int (ok));
}

std::size_t
Digest::getHash () const
{
  if (isZero ()) return 0;
  
  if (sizeof (std::size_t) > m_hashLength)
    {
      BOOST_THROW_EXCEPTION (Error::DigestCalculationError ()
                             << errmsg_info_str ("Hash is not zero and length is less than size_t")
                             << errmsg_info_int (m_hashLength));
    }
  
  // just getting first sizeof(std::size_t) bytes
  // not ideal, but should work pretty well
  return *(reinterpret_cast<std::size_t*> (m_buffer));
}

bool
Digest::operator == (const Digest &digest) const
{
  if (m_buffer == 0)
    BOOST_THROW_EXCEPTION (Error::DigestCalculationError ()
                           << errmsg_info_str ("Digest1 is empty"));

  if (digest.m_buffer == 0)
    BOOST_THROW_EXCEPTION (Error::DigestCalculationError ()
                           << errmsg_info_str ("Digest2 is empty"));

  if (m_hashLength != digest.m_hashLength)
    return false;

  // Allow different hash size
  // BOOST_THROW_EXCEPTION (Error::DigestCalculationError ()
  //                        << errmsg_info_str ("Digest lengths are not the same")
  //                        << errmsg_info_int (m_hashLength)
  //                        << errmsg_info_int (digest.m_hashLength));

  return memcmp (m_buffer, digest.m_buffer, m_hashLength) == 0;
}


void
Digest::update (const uint8_t *buffer, size_t size)
{
  // cout << "Update: " << (void*)buffer << " / size: " << size << "\n";
  
  // cannot update Digest when it has been finalized
  if (m_buffer != 0)
    BOOST_THROW_EXCEPTION (Error::DigestCalculationError ()
                           << errmsg_info_str ("Digest has been already finalized"));

  bool ok = EVP_DigestUpdate (m_context, buffer, size);
  if (!ok)
    BOOST_THROW_EXCEPTION (Error::DigestCalculationError ()
                           << errmsg_info_str ("EVP_DigestUpdate returned error")
                           << errmsg_info_int (ok));
}


Digest &
Digest::operator << (const Digest &src)
{
  if (src.m_buffer == 0) 
    BOOST_THROW_EXCEPTION (Error::DigestCalculationError ()
                           << errmsg_info_str ("Digest has not been yet finalized"));

  update (src.m_buffer, src.m_hashLength);

  return *this;
}

std::ostream &
operator << (std::ostream &os, const Digest &digest)
{
  BOOST_ASSERT (digest.m_hashLength != 0);
  
  ostreambuf_iterator<char> out_it (os); // ostream iterator
  // need to encode to base64
  copy (string_from_binary (reinterpret_cast<const char*> (digest.m_buffer)),
        string_from_binary (reinterpret_cast<const char*> (digest.m_buffer+digest.m_hashLength)),
        out_it);

  return os;
}

std::istream &
operator >> (std::istream &is, Digest &digest)
{
  string str;
  is >> str; // read string first

  if (str.size () == 0)
    BOOST_THROW_EXCEPTION (Error::DigestCalculationError ()
                           << errmsg_info_str ("Input is empty"));
  
  // uint8_t padding = (3 - str.size () % 3) % 3;
  // for (uint8_t i = 0; i < padding; i++) str.push_back ('=');

  // only empty digest object can be used for reading
  if (digest.m_buffer != 0)
    BOOST_THROW_EXCEPTION (Error::DigestCalculationError ()
                           << errmsg_info_str ("Digest has been already finalized"));

  digest.m_buffer = new uint8_t [EVP_MAX_MD_SIZE];
  uint8_t *end = copy (string_to_binary (str.begin ()),
                       string_to_binary (str.end ()),
                       digest.m_buffer);

  digest.m_hashLength = end - digest.m_buffer;

  return is;
}


} // Sync

