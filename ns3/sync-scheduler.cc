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

INIT_LOGGER ("Scheduler");

namespace Sync {

Scheduler::Scheduler ()
{
}

Scheduler::~Scheduler ()
{
}

void
Scheduler::schedule (const boost::posix_time::time_duration &reltime, Event event, uint32_t label)
{
  ns3::Seconds (reltime.total_seconds()) + ns3::MilliSeconds (reltime.milliseconds ()) + ns3::MicroSeconds (reltime.microseconds ());
}

void
Scheduler::cancel (uint32_t label)
{
  // {
  //   // cout << "Canceling label " << label << " size: " << m_events.size () << endl;
  //   lock_guard<mutex> lock (m_eventsMutex);
  //   m_events.get<byLabel> ().erase (label);
  //   // cout << "Canceled label " << label << " size: " << m_events.size () << endl;
  // }
  // m_eventsCondition.notify_one ();
  // m_thread.interrupt (); // interrupt sleep, if currently sleeping
}


} // Sync
