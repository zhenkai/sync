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
#include <boost/function.hpp>
#include "boost/date_time/posix_time/posix_time_types.hpp"

#include "sync-ccnx-wrapper.h"
#include "sync-interest-table.h"
#include "sync-diff-state.h"
#include "sync-full-state.h"
#include "sync-std-name-info.h"

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
  typedef boost::function<void (const std::string &, uint32_t, uint32_t)> LogicCallback;

  /**
   * @brief Constructor
   * @param syncPrefix the name prefix to use for the Sync Interest
   * @param fetch the fetch function, which will be called to actually fetch
   * @param ccnxHandle ccnx handle
   * the app data when new remote names are learned
   */
  SyncLogic (const std::string &syncPrefix, LogicCallback fetch, CcnxWrapperPtr ccnxHandle);

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

#ifdef _DEBUG
  size_t
  getListChecksSize ()
  {
    boost::lock_guard<boost::mutex> lock (m_listChecksMutex);
    return m_listChecks.size ();
  }
#endif
  
private:
  void delayedChecksLoop ();

  void
  processSyncInterest (DigestConstPtr digest, const std::string &interestname, bool timedProcessing=false);
  
  void sendSyncInterest ();
  // void checkAgain (const std::string &interest, DigestPtr digest);

private:
  typedef std::list< boost::tuple< boost::system_time, boost::function< void ( ) > > > DelayedChecksList;

  FullState m_state;
  DiffStateContainer m_log;
  SyncInterestTable m_syncInterestTable;

  std::string m_syncPrefix;
  LogicCallback m_fetch;
  CcnxWrapperPtr m_ccnxHandle;

  boost::thread m_delayedCheckThread;
  bool          m_delayedCheckThreadRunning;
  DelayedChecksList m_listChecks;
  boost::condition_variable m_listChecksCondition;
  boost::mutex  m_listChecksMutex;

  static const boost::posix_time::time_duration m_delayedCheckTime;
  static const int m_syncResponseFreshness = 2;
};


} // Sync

#endif // SYNC_APP_WRAPPER_H
