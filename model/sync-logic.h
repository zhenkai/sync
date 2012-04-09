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
 *         Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 */

#ifndef SYNC_LOGIC_H
#define SYNC_LOGIC_H

#include <boost/shared_ptr.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/random.hpp>
#include <memory>

#include "sync-ccnx-wrapper.h"
#include "sync-interest-table.h"
#include "sync-diff-state.h"
#include "sync-full-state.h"
#include "sync-std-name-info.h"
#include "sync-scheduler.h"

#include "sync-diff-state-container.h"

#ifdef _DEBUG
#ifdef HAVE_LOG4CXX
#include <log4cxx/logger.h>
#endif
#endif

#ifdef NS3_MODULE
#include <ns3/application.h>
#include <ns3/random-variable.h>
#endif

namespace Sync {

/**
 * \ingroup sync
 * @brief A wrapper for SyncApp, which handles ccnx related things (process
 * interests and data)
 */
class SyncLogic
#ifdef NS3_MODULE
  : public ns3::Application
#endif
{
public:
  typedef boost::function< void ( const std::string &/*prefix*/, const SeqNo &/*newSeq*/, const SeqNo &/*oldSeq*/ ) > LogicUpdateCallback;
  typedef boost::function< void ( const std::string &/*prefix*/ ) > LogicRemoveCallback;

  /**
   * @brief Constructor
   * @param syncPrefix the name prefix to use for the Sync Interest
   * @param onUpdate function that will be called when new state is detected
   * @param onRemove function that will be called when state is removed
   * @param ccnxHandle ccnx handle
   * the app data when new remote names are learned
   */
  SyncLogic (const std::string &syncPrefix,
             LogicUpdateCallback onUpdate,
             LogicRemoveCallback onRemove);

  ~SyncLogic ();

  /**
   * a wrapper for the same func in SyncApp
   */
  void addLocalNames (const std::string &prefix, uint32_t session, uint32_t seq);

  /**
   * @brief respond to the Sync Interest; a lot of logic needs to go in here
   * @param interest the Sync Interest in string format
   */
  void respondSyncInterest (const std::string &interest);

  /**
   * @brief process the fetched sync data
   * @param name the data name
   * @param dataBuffer the sync data
   */
  void processSyncData (const std::string &name, const std::string &dataBuffer);

  /**
   * @brief remove a participant's subtree from the sync tree
   * @param prefix the name prefix for the participant
   */
  void remove (const std::string &prefix);

#ifdef _DEBUG
  Scheduler &
  getScheduler () { return m_scheduler; }
#endif

protected:
#ifdef NS3_MODULE
  virtual void StartApplication ();
  virtual void StopApplication ();
#endif
  
private:
  void
  delayedChecksLoop ();

  void
  processSyncInterest (DigestConstPtr digest, const std::string &interestname, bool timedProcessing=false);
  
  void
  sendSyncInterest ();

  void 
  insertToDiffLog (DiffStatePtr diff);

  void
  satisfyPendingSyncInterests (DiffStatePtr diff);

private:
  FullState m_state;
  DiffStateContainer m_log;
  boost::recursive_mutex m_stateMutex;

  std::string m_outstandingInterest;
  SyncInterestTable m_syncInterestTable;

  std::string m_syncPrefix;
  LogicUpdateCallback m_onUpdate;
  LogicRemoveCallback m_onRemove;
  std::auto_ptr<CcnxWrapper> m_ccnxHandle;

  Scheduler m_scheduler;

#ifndef NS3_MODULE
  boost::mt19937 m_randomGenerator;
  boost::variate_generator<boost::mt19937&, boost::uniform_int<> > m_rangeUniformRandom;
#else
  ns3::UniformVariable m_rangeUniformRandom;
#endif
  
  static const int m_syncResponseFreshness = 2;

  enum EventLabels
    {
      DELAYED_INTEREST_PROCESSING = 1,
      REEXPRESSING_INTEREST = 2
    };

#ifdef _DEBUG
#ifdef HAVE_LOG4CXX
  log4cxx::LoggerPtr staticModuleLogger;
#endif
#endif  
};


} // Sync

#endif // SYNC_APP_WRAPPER_H
