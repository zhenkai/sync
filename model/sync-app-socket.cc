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

SyncAppSocket::SyncAppSocket(string syncPrefix, function<void (string)> dataCallback)
{
  m_ccnxHandle.reset(new CcnxWrapper());
  m_fetcher = new AppDataFetch(m_ccnxHandle, dataCallback);
  m_publisher = new AppDataPublish(m_ccnxHandle);

  function<void (string, uint32_t, uint32_t)> f(bind(&AppDataFetch::fetch, m_fetcher, _1, _2, _3));
  m_syncLogic = new SyncLogic(syncPrefix, f, m_ccnxHandle);
}

SyncAppSocket::~SyncAppSocket()
{
  delete m_syncLogic;
  delete m_fetcher;
  delete m_publisher;
}

bool SyncAppSocket::publish(string prefix, string dataBuffer, int freshness)
{
  m_publisher->publishData(prefix, dataBuffer, freshness);
  m_syncLogic->addLocalNames(prefix, m_publisher->getHighestSeq(prefix));
}

}