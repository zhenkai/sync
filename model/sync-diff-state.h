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

#ifndef SYNC_DIFF_STATE_H
#define SYNC_DIFF_STATE_H

#include "sync-state.h"

namespace Sync {

class DiffState;
typedef boost::shared_ptr<DiffState> DiffStatePtr;
typedef boost::shared_ptr<DiffState> DiffStateConstPtr;

/**
 * @ingroup ccnx
 * @brief Differential SYNC state
 */
class DiffState : public State
{
public:
  /**
   * @see Default constructor
   */
  DiffState ();
  virtual ~DiffState ();

  /**
   * @brief Set successor for the diff state
   * @param next successor state
   */
  void
  setNext (DiffStatePtr next)
  {
    m_next = next;
  }

  // void
  // setDigest (Hash digest);

  // Hash
  // getDigest () const;

  /**
   * @brief Accumulate differences from `this' state to the most current state
   * @returns Accumulated differences from `this' state to the most current state
   */
  DiffStatePtr
  diff () const;

  /**
   * @brief Combine differences from `this' and `state'
   * @param state Differential state to combine with
   * @return Combined differences
   *
   * In case of collisions, `this' leaf will be replaced with the leaf of `state'
   */
  DiffState&
  operator += (const DiffState &state);
  
  // from State
  virtual void
  update (NameInfoConstPtr info, const SeqNo &seq);

  virtual void
  remove (NameInfoConstPtr info);
  
private:
  DiffStatePtr m_next;
  // Hash m_digest;
};

} // Sync

#endif // SYNC_DIFF_STATE_H
