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

string AppDataPublish::getRecentData(string prefix, uint32_t session)
{

}

uint32_t AppDataPublish::getHighestSeq(string prefix, uint32_t session)
{
  unordered_map<string, Seq>::iterator i = m_sequenceLog.find(prefix);

  if (i != m_sequenceLog.end())
  {
    Seq s = i->second;
    if (s.session == session)
      return s.seq;
  }

  return 0;
}

bool AppDataPublish::publishData(string name, uint32_t session, string dataBuffer, int freshness)
{
  uint32_t seq = getHighestSeq(name, session);
  if (seq == 0)
    m_sequenceLog.erase(name);

  seq++;
  if (seq == 0)
    seq = 1;
  Seq s;
  s.session = session;
  s.seq = seq;
  m_sequenceLog[name] = s;

  string contentName = name;
  contentName += seq;

  m_ccnxHandle->publishData(contentName, dataBuffer, freshness);

  return true;
}

}