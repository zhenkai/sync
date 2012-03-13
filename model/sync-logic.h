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
 *         Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 */

#ifndef SYNC_LOGIC_H
#define SYNC_LOGIC_H

#include <boost/shared_ptr.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/random.hpp>

#include "sync-ccnx-wrapper.h"
#include "sync-interest-table.h"
#include "sync-diff-state.h"
#include "sync-full-state.h"
#include "sync-std-name-info.h"
#include "sync-scheduler.h"

#include "sync-diff-state-container.h"

namespace Sync {

/**
 * \ingroup sync
 * @brief A wrapper for SyncApp, which handles ccnx related things (process
 * interests and data)
 */
class SyncLogic
{
public:
  typedef boost::function< void ( const std::string &/*prefix*/, const SeqNo &/*newSeq*/, const SeqNo &/*oldSeq*/ ) > LogicUpdateCallback;
  typedef boost::function< void ( const std::string &/*prefix*/ ) > LogicRemoveCallback;

  /**
   * @brief Constructor
   * @param syncPrefix the name prefix to use for the Sync Interest
   * @param fetch the fetch function, which will be called to actually fetch
   * @param ccnxHandle ccnx handle
   * the app data when new remote names are learned
   */
  SyncLogic (const std::string &syncPrefix,
             LogicUpdateCallback onUpdate,
             LogicRemoveCallback onRemove,
             CcnxWrapperPtr ccnxHandle);

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
   * @param the name prefix for the participant
   */
  void remove(const std::string &prefix);

#ifdef _DEBUG
  Scheduler &
  getScheduler () { return m_delayedChecksScheduler; }
#endif
  
private:
  void
  delayedChecksLoop ();

  void
  processSyncInterest (DigestConstPtr digest, const std::string &interestname, bool timedProcessing=false);
  
  void
  sendSyncInterest ();

  void 
  processPendingSyncInterests(DiffStatePtr &diff);

private:
  FullState m_state;
  DiffStateContainer m_log;
  boost::recursive_mutex m_stateMutex;

  SyncInterestTable m_syncInterestTable;

  std::string m_syncPrefix;
  LogicUpdateCallback m_onUpdate;
  LogicRemoveCallback m_onRemove;
  CcnxWrapperPtr m_ccnxHandle;

  Scheduler m_delayedChecksScheduler;

  boost::mt19937 m_randomGenerator;
  boost::variate_generator<boost::mt19937&, boost::uniform_int<> > m_rangeUniformRandom;
  
  static const int m_syncResponseFreshness = 2;
};


} // Sync

#endif // SYNC_APP_WRAPPER_H
