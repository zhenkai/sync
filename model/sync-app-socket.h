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

#ifndef SYNC_APP_SOCKET_H
#define SYNC_APP_SOCKET_H

#include "sync-logic.h"
#include "sync-app-data-fetch.h"
#include "sync-app-data-publish.h"
#include <boost/function.hpp>

namespace Sync {

/**
 * \ingroup sync
 * @brief A simple interface to interact with client code
 */
class SyncAppSocket
{
public:
  /**
   * @brief the constructor for SyncAppSocket; the parameter syncPrefix
   * should be passed to the constructor of m_syncAppWrapper; the other
   * parameter should be passed to the constructor of m_fetcher; furthermore,
   * the fetch function of m_fetcher should be a second paramter passed to
   * the constructor of m_syncAppWrapper, so that m_syncAppWrapper can tell
   * m_fetcher to fetch the actual app data after it learns the names
   *
   * @param syncPrefix the name prefix for Sync Interest
   * @param dataCallback the callback to process data
   */
  SyncAppSocket (const std::string &syncPrefix, CcnxWrapper::DataCallback dataCallback);
  ~SyncAppSocket ();

  /**
   * @brief publish data from local client and tell SyncLogic to update
   * the sync tree by adding the local names
   *
   * @param prefix the name prefix for the data
   * @param session session to which data is published
   * @param dataBuffer the data itself
   * @param freshness the freshness time for the data (in seconds)
   */
  bool publish (const std::string &prefix, uint32_t session, const std::string &dataBuffer, int freshness);

  /**
   * @brief delete a participant's subtree from the sync tree; SyncLogic will do the work
   * this is just a wrapper
   *
   * @param prefix the prefix for the participant
   */
  void remove (const std::string &prefix) {m_syncLogic.remove(prefix);}

private:
	CcnxWrapperPtr m_appHandle;

  AppDataFetch   m_fetcher;
  AppDataPublish m_publisher;
  SyncLogic      m_syncLogic;
};

} // Sync

#endif // SYNC_APP_SOCKET_H
