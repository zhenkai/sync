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
#include <map>

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

struct MissingDataInfo {
  std::string prefix;
  SeqNo low;
  SeqNo high;
};

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
  //typedef boost::function< void ( const std::string &/*prefix*/, const SeqNo &/*newSeq*/, const SeqNo &/*oldSeq*/ ) > LogicUpdateCallback;
  typedef boost::function< void (const std::vector<MissingDataInfo> & ) > LogicUpdateCallback;
  typedef boost::function< void ( const std::string &/*prefix*/ ) > LogicRemoveCallback;
  typedef boost::function< void (const std::string &)> LogicPerBranchCallback;

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

  SyncLogic (const std::string &syncPrefix,
             LogicPerBranchCallback onUpdateBranch);

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
  void respondSyncData (const std::string &name, const char *wireData, size_t len);

  /**
   * @brief remove a participant's subtree from the sync tree
   * @param prefix the name prefix for the participant
   */
  void remove (const std::string &prefix);

  std::string
  getRootDigest();

#ifdef _DEBUG
  Scheduler &
  getScheduler () { return m_scheduler; }
#endif

#ifdef NS3_MODULE
public:
  virtual void StartApplication ();
  virtual void StopApplication ();
#endif
  
  void stop();

  void
  printState () const;

  std::map<std::string, bool>
  getBranchPrefixes() const;

private:
  void
  delayedChecksLoop ();

  void
  processSyncInterest (const std::string &name,
                       DigestConstPtr digest, bool timedProcessing=false);

  void
  processSyncData (const std::string &name,
                   DigestConstPtr digest, const char *wireData, size_t len);
  
  void
  processSyncRecoveryInterest (const std::string &name,
                               DigestConstPtr digest);
  
  void 
  insertToDiffLog (DiffStatePtr diff);

  void
  satisfyPendingSyncInterests (DiffStateConstPtr diff);

  boost::tuple<DigestConstPtr, std::string>
  convertNameToDigestAndType (const std::string &name);

  void
  sendSyncInterest ();

  void
  sendSyncRecoveryInterests (DigestConstPtr digest);

  void
  sendSyncData (const std::string &name,
                DigestConstPtr digest, StateConstPtr state);

  void
  sendSyncData (const std::string &name,
                DigestConstPtr digest, SyncStateMsg &msg);

  size_t
  getNumberOfBranches () const;
  
private:
  FullStatePtr m_state;
  DiffStateContainer m_log;
  mutable boost::recursive_mutex m_stateMutex;

  std::string m_outstandingInterestName;
  SyncInterestTable m_syncInterestTable;

  std::string m_syncPrefix;
  LogicUpdateCallback m_onUpdate;
  LogicRemoveCallback m_onRemove;
  LogicPerBranchCallback m_onUpdateBranch;
  bool m_perBranch;
  CcnxWrapperPtr m_ccnxHandle;

  Scheduler m_scheduler;

#ifndef NS3_MODULE
  boost::mt19937 m_randomGenerator;
  boost::variate_generator<boost::mt19937&, boost::uniform_int<> > m_rangeUniformRandom;
  boost::variate_generator<boost::mt19937&, boost::uniform_int<> > m_reexpressionJitter;
#else
  ns3::UniformVariable m_rangeUniformRandom;
  ns3::UniformVariable m_reexpressionJitter;
#endif

  static const int m_unknownDigestStoreTime = 10; // seconds
#ifdef NS3_MODULE
  static const int m_syncResponseFreshness = 100; // milliseconds
  static const int m_syncInterestReexpress = 10; // seconds
  // don't forget to adjust value in SyncCcnxWrapper
#else
  static const int m_syncResponseFreshness = 4;
  static const int m_syncInterestReexpress = 4;
#endif

  static const int m_defaultRecoveryRetransmitInterval = 200; // milliseconds
  uint32_t m_recoveryRetransmissionInterval; // milliseconds
  
  enum EventLabels
    {
      DELAYED_INTEREST_PROCESSING = 1,
      REEXPRESSING_INTEREST = 2,
      REEXPRESSING_RECOVERY_INTEREST = 3
    };
};


} // Sync

#endif // SYNC_APP_WRAPPER_H
