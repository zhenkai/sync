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

#include "sync-app-socket.h"

using namespace std;
using namespace boost;

namespace Sync
{

SyncAppSocket::SyncAppSocket (const string &syncPrefix, CcnxWrapper::DataCallback dataCallback)
  : m_ccnxHandle (new CcnxWrapper())
  , m_fetcher (m_ccnxHandle, dataCallback)
  , m_publisher (m_ccnxHandle)
  , m_syncLogic (syncPrefix,
                 bind (&AppDataFetch::fetch, m_fetcher, _1, _2, _3),
                 m_ccnxHandle)
{
}

SyncAppSocket::~SyncAppSocket()
{
}

bool SyncAppSocket::publish (const string &prefix, uint32_t session, const string &dataBuffer, int freshness)
{
  m_publisher.publishData (prefix, session, dataBuffer, freshness);
  m_syncLogic.addLocalNames (prefix, session, m_publisher.getHighestSeq (prefix, session));
}

}
