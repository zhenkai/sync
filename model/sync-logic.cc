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

#include "sync-logic.h"

using namespace std;
using namespace boost;

namespace Sync
{

SyncLogic::SyncLogic(string syncPrefix,
			       function<void (string, uint32_t, uint32_t)> fetch,
			       shared_ptr<CcnxWrapper> ccnxHandle)
{
  m_syncPrefix = syncPrefix;
  m_fetch = fetch;
  m_ccnxHandle = ccnxHandle;
}

SyncLogic::~SyncLogic()
{

}

void SyncLogic::processSyncData(string dataBuffer)
{

}

void SyncLogic::addLocalNames(string prefix, uint32_t seq)
{

}

void SyncLogic::respondSyncInterest(string interest)
{

}

void SyncLogic::sendSyncInterest()
{

}

}