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
 *         卞超轶 Chaoyi Bian <bcy@pku.edu.cn>
 *	   Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 */

#include "sync-digest.h"
#include <string.h>

#include "ns3/assert.h"
#include <boost/exception/errinfo_at_line.hpp>

using namespace boost;

namespace ns3 {
namespace Sync {

Digest::Digest ()
  : m_buffer (0)
  , m_hashLength (0)
{
  m_context = EVP_MD_CTX_create ();

  int ok = EVP_DigestInit_ex (m_context, EVP_sha1 (), 0);
  if (!ok)
    throw DigestCalculationError () << errinfo_at_line (__LINE__);
}

Digest::~Digest ()
{
  if (m_buffer != 0)
    delete [] m_buffer;

  EVP_MD_CTX_destroy (m_context);
}

void
Digest::Finalize ()
{
  if (m_buffer != 0) return;

  m_buffer = new uint8_t [HASH_SIZE];

  int ok = EVP_DigestFinal_ex (m_context,
			       m_buffer, &m_hashLength);
  if (!ok)
    throw DigestCalculationError () << errinfo_at_line (__LINE__);
}
  
std::size_t
Digest::getHash ()
{
  if (m_buffer == 0)
    Finalize ();

  NS_ASSERT (sizeof (std::size_t) <= m_hashLength);
  
  // just getting first sizeof(std::size_t) bytes
  // not ideal, but should work pretty well
  return reinterpret_cast<std::size_t> (m_buffer);
}

bool
Digest::operator == (Digest &digest)
{
  if (m_buffer == 0)
    Finalize ();

  if (digest.m_buffer == 0)
    digest.Finalize ();
  
  NS_ASSERT (m_hashLength == digest.m_hashLength);

  return memcmp (m_buffer, digest.m_buffer, m_hashLength) == 0;
}


Digest &
Digest::operator << (const Digest &src)
{
  if (src.m_buffer == 0)
    throw DigestCalculationError () << errinfo_at_line (__LINE__);

  bool ok = EVP_DigestUpdate (m_context, src.m_buffer, src.m_hashLength);
  if (!ok)
    throw DigestCalculationError () << errinfo_at_line (__LINE__);
  
  return *this;
}


} // Sync
} // ns3

