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

#ifndef SYNC_APP_DATA_FETCH_H
#define SYNC_APP_DATA_FETCH_H

#include "sync-ccnx-wrapper.h"
#include "sync-seq-no.h"

namespace Sync {

/**
 * \ingroup sync
 * @brief fetch application data, by default it will try to fetch every piece
 * of data
 */
class AppDataFetch
{
public:
  /**
   * @brief Constructor
   * @param ccnxHandle handle for CCNx
   * @param dataCallback the callback function to process data
   */
  AppDataFetch (CcnxWrapperPtr ccnxHandle,
                CcnxWrapper::DataCallback dataCallback)
    : m_ccnxHandle (ccnxHandle)
    , m_dataCallback (dataCallback)
  { }

  /**
   * @brief Set data callback
   * @param dataCallback the callback function to process data
   */
  void
  setDataCallback(CcnxWrapper::DataCallback dataCallback)
  {
    m_dataCallback = dataCallback;
  }

  /**
   * @brief Fired from SyncLogic when new data is available
   *
   * @param prefix the prefix for the data
   * @param newSeq old session ID/sequence number
   * @param oldSeq new session ID/sequence number
   */
  void
  onUpdate (const std::string &prefix, const SeqNo &newSeq, const SeqNo &oldSeq);

  /**
   * @brief Fired from SyncLogic when data is removed
   *
   * @param prefix the prefix for the data
   */
  void
  onRemove (const std::string &prefix);
  
private:
  CcnxWrapperPtr m_ccnxHandle;
  CcnxWrapper::DataCallback m_dataCallback;
};


} // Sync

#endif // SYNC_APP_DATA_FETCH_H
