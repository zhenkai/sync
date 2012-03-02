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
  virtual void
  update (NameInfoConstPtr info, const SeqNo &seq) = 0;

  /**
   * @brief Remove leaf from the state tree
   *
   * @param info name of the leaf
   */
  virtual void
  remove (NameInfoConstPtr info) = 0;

protected:
  LeafContainer m_leaves;
};

} // Sync

#endif // SYNC_STATE_H
