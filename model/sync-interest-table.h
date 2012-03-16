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

#ifndef SYNC_INTEREST_TABLE_H
#define SYNC_INTEREST_TABLE_H
#include <string>
#include <vector>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/thread.hpp>
#include <ctime>
#include "sync-scheduler.h"

namespace Sync {

/**
 * \ingroup sync
 * @brief A table to keep unanswered Sync Interest
 * all access operation to the table should grab the 
 * mutex first
 */
class SyncInterestTable
{
public:
  SyncInterestTable ();
  ~SyncInterestTable ();

  /**
   * @brief Insert an interest, if interest already exists, update the
   * timestamp
   */
  bool
  insert (const std::string &interest);

  /**
   * @brief Remove interest (e.g., when it was satisfied)
   */
  bool
  remove (const std::string &interest);

  /**
   * @brief fetch all Interests and clear the table
   */
  std::vector<std::string>
  fetchAll ();

  uint32_t
  size () const;

private:
  /**
   * @brief periodically called to expire Interest
   */
  void
  expireInterests ();

private:
  typedef boost::unordered_map<std::string, time_t> TableContainer;
  
  static const int m_checkPeriod = 4;
  TableContainer m_table; // pit entries

  Scheduler m_scheduler;
  mutable boost::recursive_mutex m_mutex;
};

} // Sync

#endif // SYNC_INTEREST_TABLE_H
