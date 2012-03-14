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

#ifndef SYNC_APP_DATA_PUBLISH_H
#define SYNC_APP_DATA_PUBLISH_H

// #include <boost/unordered_map.hpp>
#include "sync-seq-no.h"
#include "sync-ccnx-wrapper.h"
#include <utility>
#include <map>

namespace Sync {

struct Seq
{
  uint32_t session;
  uint32_t seq;
};

struct GetSeqException : virtual boost::exception, virtual std::exception { };

/**
 * \ingroup sync
 * @brief publishes application data using incrementing sequence number (for
 * each sequence namber and keeps track of most recently published data for
 * each name prefix
 */
class AppDataPublish
{
public:
  AppDataPublish (CcnxWrapperPtr ccnxHandle)
  : m_ccnxHandle (ccnxHandle)
  { }

  /**
   * @brief get the name (including sequence number) and the content
   * (unencoded, just XML stanza) of the most recent published data
   *
   * @param prefix the name prefix to look for
   * @param session session
   * @return the pair of name and content
   */
  std::string
  getRecentData (const std::string &prefix, uint32_t session);

  /**
   * @brief get the most recent sequence number for a name prefix
   * @param prefix the name prefix to look for
   * @param session session
   */
  uint32_t
  getNextSeq (const std::string &prefix, uint32_t session);

  /**
   * @brief publish data for a name prefix, updates the corresponding
   * sequence number and recent data
   *
   * @param name data prefix
   * @param session session to which data is published
   * @param dataBuffer the data itself
   * @param freshness the freshness for the data object
   * @return published sequence number, will throw an exception if something wrong happened
   */
  uint32_t publishData (const std::string &name, uint32_t session, const std::string &dataBuffer, int freshness);

private:
  typedef std::map<std::string, Seq> SequenceLog;
  typedef std::map<std::pair<std::string, uint32_t>, std::string> RecentData;
  
  CcnxWrapperPtr m_ccnxHandle;
  SequenceLog m_sequenceLog;
  RecentData m_recentData;
};

} // Sync

#endif // SYNC_APP_DATA_PUBLISH_H
