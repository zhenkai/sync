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

#ifndef SYNC_DIGEST_H
#define SYNC_DIGEST_H

#include <boost/exception/all.hpp>
#include <openssl/evp.h>
#include <boost/cstdint.hpp>

namespace ns3 {
namespace Sync {

const std::size_t HASH_SIZE = 160;

/**
 * @ingroup sync
 * @brief A simple wrapper for libcrypto hash functions
 */
class Digest
{
public:
  /**
   * @brief Default constructor.  Will initialize internal libssl structures
   */
  Digest ();

  /**
   * @brief Destructor
   */
  ~Digest ();

  // Digest &
  // operator << (

  /**
   * @brief Obtain a short version of the hash (just first sizeof(size_t) bytes
   *
   * Side effect: Finalize will be called on `this'
   */
  std::size_t
  getHash ();

  /**
   * @brief Compare two full digests
   *
   * Side effect: Finalize will be called on `this' and `digest'
   */
  bool
  operator == (Digest &digest);

private:
  /**
   * @brief Finalize digest. All subsequent calls to "operator <<" will fire an exception
   */
  void Finalize ();
  
private:
  EVP_MD_CTX *m_context;
  uint8_t *m_buffer;
  uint32_t m_hashLength;
};

struct DigestCalculationError : virtual boost::exception { };

} // Sync
} // ns3

#endif // SYNC_DIGEST_H
