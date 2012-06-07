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

using namespace boost;
using namespace std;

INIT_LOGGER ("Scheduler");

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
  _LOG_FUNCTION (this);
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

	  if (nextTime > get_system_time ())
	    {
	      this_thread::sleep (nextTime - get_system_time ());

	      // sleeping

	      if (nextTime > get_system_time ())
                {
                  // cout << "expected here" << endl;
                  continue; // something changes, try again
                }
	    }

	  if (!m_threadRunning) continue;

	  Event event;
	  
	  {
	    lock_guard<mutex> lock (m_eventsMutex);

	    if (m_events.size () == 0)
              {
                // cout << "Here" << endl;
                continue;
              }
	    
	    event = m_events.begin ()->event;
	    m_events.erase (m_events.begin ());
	  }

	  event (); // calling the event outside the locked mutex
        }
      catch (thread_interrupted &e)
        {
          // cout << "interrupted: " << this_thread::get_id () << endl;
          // do nothing
        }
    }
  // cout << "Exited...\n";  
}

void
Scheduler::schedule (const TimeDuration &reltime, Event event, uint32_t label)
{
  lock_guard<mutex> lock (m_eventsMutex);
  m_events.insert (LogicEvent (boost::get_system_time () + reltime, event, label));

  m_eventsCondition.notify_one ();
  m_thread.interrupt (); // interrupt sleep, if currently sleeping
}

void
Scheduler::cancel (uint32_t label)
{
  // cout << "Canceling label " << label << " size: " << m_events.size () << endl;
  lock_guard<mutex> lock (m_eventsMutex);
  m_events.get<byLabel> ().erase (label);
  // cout << "Canceled label " << label << " size: " << m_events.size () << endl;

  m_eventsCondition.notify_one ();
  m_thread.interrupt (); // interrupt sleep, if currently sleeping
}


} // Sync
