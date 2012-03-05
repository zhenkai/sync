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
#include <boost/foreach.hpp>
#include <boost/assert.hpp>

#include "sync-full-leaf.h"

using namespace boost;
namespace ll = boost::lambda;

namespace Sync {


FullState::FullState ()
  : m_lastUpdated (0)
{
}

FullState::~FullState ()
{
}

ns3::Time
FullState::getTimeFromLastUpdate () const
{
  return ns3::Simulator::Now () - m_lastUpdated;
}

DigestConstPtr
FullState::getDigest ()
{
  if (m_digest == 0)
    {
      m_digest = make_shared<Digest> ();
      BOOST_FOREACH (LeafConstPtr leaf, m_leaves)
        {
          FullLeafConstPtr fullLeaf = dynamic_pointer_cast<const FullLeaf> (leaf);
          BOOST_ASSERT (fullLeaf != 0);
          *m_digest << fullLeaf->getDigest ();
        }
    }

  return m_digest;
}

// from State
void
FullState::update (NameInfoConstPtr info, const SeqNo &seq)
{
  m_lastUpdated = ns3::Simulator::Now ();
  m_digest.reset ();

  LeafContainer::iterator item = m_leaves.find (*info);
  if (item == m_leaves.end ())
    {
      m_leaves.insert (make_shared<Leaf> (info, cref (seq)));
    }
  else
    {
      m_leaves.modify (item, ll::bind (&Leaf::setSeq, *ll::_1, seq));
    }
}

void
FullState::remove (NameInfoConstPtr info)
{
  m_lastUpdated = ns3::Simulator::Now ();
  m_digest.reset ();

  m_leaves.erase (*info);
}


} // Sync
