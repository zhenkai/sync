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

#include "sync-app-data-publish.h"

using namespace std;
using namespace boost;

namespace Sync
{

pair<string, string> AppDataPublish::getRecentData(string prefix)
{

}

uint32_t AppDataPublish::getHighestSeq(string prefix)
{
  unordered_map<string, uint32_t>::iterator i = m_sequenceLog.find(prefix);

  if (i != m_sequenceLog.end())
  {
    return i->second;
  }
  else
  {
    m_sequenceLog[prefix] = 0;
    return 0;
  }

}

bool AppDataPublish::publishData(string name, string dataBuffer, int freshness)
{
  uint32_t seq = getHighestSeq(name) + 1;
  string contentName = name;

  contentName += seq;

  m_sequenceLog[contentName] = seq;
  m_recentData[contentName] = dataBuffer;

  m_ccnxHandle->publishData(contentName, dataBuffer, freshness);

  return true;
}

}