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

#ifndef SYNC_STATE_H
#define SYNC_STATE_H

#include "sync-state-leaf-container.h"
#include <boost/exception/all.hpp>
#include "boost/tuple/tuple.hpp"

/**
 * \defgroup sync SYNC protocol
 *
 * Implementation of SYNC protocol
 */
namespace Sync {

/**
 * \ingroup sync
 * @brief Container for state leaves and definition of the abstract interface to work with State objects
 */
class State
{
public:
  virtual ~State () { };
  
  /**
   * @brief Add or update leaf to the state tree
   *
   * @param info name of the leaf
   * @param seq  sequence number of the leaf
   */
  virtual boost::tuple<bool/*inserted*/, bool/*updated*/, SeqNo/*oldSeqNo*/>
  update (NameInfoConstPtr info, const SeqNo &seq) = 0;

  /**
   * @brief Remove leaf from the state tree
   * @param info name of the leaf
   */
  virtual bool
  remove (NameInfoConstPtr info) = 0;

  /**
   * @brief Get state leaves
   */
  const LeafContainer &
  getLeaves () const 
  { return m_leaves; }
  
protected:
  LeafContainer m_leaves;
};

/**
 * @brief Formats an XML representation of the state
 * @param os output stream
 * @param state state
 * @returns output stream
 */
std::ostream &
operator << (std::ostream &os, const State &state);

/**
 * @brief Parses an XML representation to the state
 * @param in input data stream
 * @param state state
 * @returns input stream
 */
std::istream &
operator >> (std::istream &in, State &state);

namespace Error {
/**
 * @brief Will be thrown when XML cannot be properly decoded to State
 */
struct SyncXmlDecodingFailure : virtual boost::exception, virtual std::exception { };
}

} // Sync

#endif // SYNC_STATE_H
