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

#ifndef SYNC_SCHEDULER_H
#define SYNC_SCHEDULER_H

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

#include "sync-logic-event-container.h"

#define TIME_SECONDS(number) boost::posix_time::seconds (number)
#define TIME_MILLISECONDS(number) boost::posix_time::milliseconds(number)
#define TIME_NOW boost::get_system_time ()
typedef boost::posix_time::time_duration TimeDuration;
typedef boost::system_time TimeAbsolute;

namespace Sync {

/**
 * @ingroup sync
 * @brief General purpose event scheduler
 *
 * This class internally runs a thread and events can be scheduled by specifying an absolute or relative time of the event
 */
class Scheduler
{
public:
  /**
   * @brief Default constructor. Thread will be created
   */
  Scheduler ();
  /**
   * @brief Destructor. Thread will be nicely stopped
   */
  ~Scheduler ();

  /**
   * @brief Schedule an event at relative time 'reltime'
   * @param reltime Relative time
   * @param event function to be called at the time
   * @param label Label for the event
   */
  void
  schedule (const TimeDuration &reltime, Event event, uint32_t label); 

  /**
   * @brief Cancel all events for the label
   * @param label Label of the event that needs to be cancelled
   */
  void
  cancel (uint32_t label);

#ifdef _DEBUG
  size_t
  getEventsSize ()
  {
    boost::lock_guard<boost::mutex> lock (m_eventsMutex);
    return m_events.size ();
  }
#endif
  
private:
  void
  threadLoop ();
    
private:
  EventsContainer m_events;
  boost::condition_variable m_eventsCondition;
  boost::mutex  m_eventsMutex;

  boost::thread m_thread;
  bool          m_threadRunning;
};
  
}

#endif // SYNC_SCHEDULER_H
