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

#include "sync-diff-state.h"

#include <boost/make_shared.hpp>

using namespace boost;

namespace ns3 {
namespace Sync {

DiffState::DiffState ()
  : m_next (0)
{
}

DiffState::~DiffState ()
{
}

DiffStatePtr
DiffState::diff () const
{
  DiffStatePtr ret = make_shared<DiffState> ();

  DiffState *state = m_next;
  while (state != 0)
    {
      *ret += *state;
      state = state->m_next;
    }

  return ret;
}

DiffState &
DiffState::operator += (const DiffState &state)
{
  return *this;
}
  
// from State
virtual void
DiffState::update (NameInfoConstPtr info, const SeqNo &seq)
{
  LeafContainer::iterator item = m_leafs.find (info);
  if (item != m_leafs.end ())
    m_leafs.remove (item);
  
  m_leafs.insert (make_shared<DiffLeaf> (info, cref (seq)));
}

virtual void
DiffState::remove (NameInfoConstPtr info)
{
  LeafContainer::iterator item = m_leafs.find (info);
  if (item != m_leafs.end ())
    m_leafs.remove (item);

  m_leafs.insert (make_shared<DiffLeaf> (info));
}


} // ns3
} // Sync
