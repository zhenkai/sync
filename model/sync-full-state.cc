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

#include "sync-full-state.h"

#include "ns3/simulator.h"

#include <boost/make_shared.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

using namespace boost;
namespace ll = boost::lambda;

namespace ns3 {
namespace Sync {


FullState::FullState ()
  : m_lastUpdated (0)
{
}

FullState::~FullState ()
{
}

Time
FullState::getTimeFromLastUpdate () const
{
  return Simulator::Now () - m_lastUpdated;
}
  
// from State
void
FullState::update (NameInfoConstPtr info, const SeqNo &seq)
{
  m_lastUpdated = Simulator::Now ();

  LeafContainer::iterator item = m_leafs.find (*info);
  if (item == m_leafs.end ())
    {
      m_leafs.insert (make_shared<Leaf> (info, cref (seq)));
    }
  else
    {
      m_leafs.modify (item, ll::bind (&Leaf::setSeq, ll::_1, cref (seq)));
    }
}

void
FullState::remove (NameInfoConstPtr info)
{
  m_lastUpdated = Simulator::Now ();

  m_leafs.remove (info);
}


} // Sync
} // ns3
