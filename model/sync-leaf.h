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

#ifndef SYNC_LEAF_H
#define SYNC_LEAF_H

#include "sync-seq-no.h"
#include <boost/shared_ptr.hpp>

namespace ns3 {
namespace Sync {

class NameInfo;

/**
 * \ingroup sync
 * @brief Sync tree leaf
 */
class Leaf
{
public:
  /**
   * @brief Constructor
   * @param info Smart pointer to leaf's name
   * @param seq  Initial sequence number of the pointer
   */
  Leaf (boost::shared_ptr<const NameInfo> info, const SeqNo &seq)
    : m_info (info)
    , m_seq (seq)
  { }

  virtual ~Leaf ()
  {
  }
  
  /**
   * @brief Get name of the leaf
   */
  const NameInfo &
  getInfo () const
  {
    return *m_info;
  }

  /**
   * @brief Get sequence number of the leaf
   */
  const SeqNo&
  getSeq () const
  {
    return m_seq;
  }

  /**
   * @brief Update sequence number of the leaf
   * @param seq Sequence number
   *
   * Sequence number is updated to the largest value among this->m_seq and seq
   */
  void
  setSeq (const SeqNo &seq)
  {
    m_seq = std::max (m_seq, seq);
  }
  
private:
  NameInfoConstPtr m_info;
  SeqNo m_seq;
};

typedef boost::shared_ptr<Leaf> LeafPtr;

} // Sync
} // ns3

#endif // SYNC_LEAF_H
