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

#include "sync-scheduler.h"
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace boost;
using namespace std;

namespace Sync {

Scheduler::Scheduler ()
  : m_threadRunning (true)
{
  m_thread = thread (&Scheduler::threadLoop, this);
}

Scheduler::~Scheduler ()
{
  m_threadRunning = false;
  // cout << "Requested stop" << this_thread::get_id () << endl;
  m_thread.interrupt ();
  m_thread.join ();
}

void
Scheduler::threadLoop ()
{
  while (m_threadRunning)
    {
      try
        {
	  boost::system_time nextTime;
          {
            unique_lock<mutex> lock (m_eventsMutex);
            while (m_threadRunning && m_events.size () == 0)
              {
                m_eventsCondition.wait (lock);
                // cout << "Got something" << endl;
              }

            if (m_events.size () == 0) continue;

	    nextTime = m_events.begin ()->time;
          }

	  if (nextTime - get_system_time () > posix_time::time_duration (0,0,0,0))
	    {
	      this_thread::sleep (nextTime - get_system_time ());

	      // sleeping

	      if (nextTime - get_system_time () > posix_time::time_duration (0,0,0,0))
		continue; // something changes, try again
	    }

	  if (!m_threadRunning) continue;

	  Event event;
	  
	  {
	    lock_guard<mutex> lock (m_eventsMutex);

	    BOOST_ASSERT (m_events.size () != 0);
	    
	    event = m_events.begin ()->event;
	    m_events.erase (m_events.begin ());
	  }

	  event (); // calling the event outside the locked mutex
        }
      catch (thread_interrupted e)
        {
          // cout << "interrupted: " << this_thread::get_id () << endl;
          // do nothing
        }
    }
  // cout << "Exited...\n";  
}


void
Scheduler::schedule (const boost::system_time &abstime, Event event)
{
  {
    lock_guard<mutex> lock (m_eventsMutex);
    m_events.insert (LogicEvent (abstime, event));
  }
  m_eventsCondition.notify_one ();
  m_thread.interrupt (); // interrupt sleep, if currently sleeping
}

void
Scheduler::schedule (const boost::posix_time::time_duration &reltime, Event event)
{
  // cout << reltime << endl;
  schedule (boost::get_system_time () + reltime, event);
}

} // Sync
