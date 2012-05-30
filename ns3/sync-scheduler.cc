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

#include "sync-scheduler.h"
#include "sync-log.h"

#include "ns3/simulator.h"

using namespace boost;
using namespace std;
using namespace ns3;

INIT_LOGGER ("Scheduler");

namespace Sync {

Scheduler::Scheduler ()
{
}

Scheduler::~Scheduler ()
{
}

void
Scheduler::eventWrapper (Event event)
{
  event ();
}

void
Scheduler::schedule (const TimeDuration &reltime, Event event, uint32_t label)
{
  _LOG_DEBUG ("Schedule event for " << (Simulator::Now () +reltime).ToDouble (Time::S) << "s for label " << label);
  
  list< EventId > &eventsForLabel = m_labeledEvents [label];
  list< EventId >::iterator i = eventsForLabel.begin ();
  while (i != eventsForLabel.end ())
    {
      if (i->IsExpired ())
        {
          list< EventId >::iterator next = i;
          next ++;
          eventsForLabel.erase (i);
          i = next;
        }
      else
        i ++;
    }

  ns3::EventId eventId = ns3::Simulator::Schedule (reltime, Scheduler::eventWrapper, event);
  eventsForLabel.push_back (eventId);
}

void
Scheduler::cancel (uint32_t label)
{
  list< EventId > &eventsForLabel = m_labeledEvents [label];
  _LOG_DEBUG ("Canceling events for label " << label << " (" << eventsForLabel.size () << " events)");

  for (list< EventId >::iterator i = eventsForLabel.begin ();
       i != eventsForLabel.end ();
       i++)
    {
      i->Cancel ();
    }

  eventsForLabel.clear ();
}


} // Sync
