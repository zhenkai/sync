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
#include <boost/throw_exception.hpp>
typedef boost::error_info<struct tag_errmsg, std::string> errmsg_info_str;
typedef boost::error_info<struct tag_errmsg, int> errmsg_info_int;

using namespace std;
using namespace boost;

namespace Sync
{


string
AppDataPublish::getRecentData (const string &prefix, uint32_t session)
{
  if (m_recentData.find(make_pair(prefix, session)) != m_recentData.end())
    return m_recentData[make_pair(prefix, session)];
  else
    return "";
}

uint32_t
AppDataPublish::getHighestSeq (const string &prefix, uint32_t session)
{
  unordered_map<string, Seq>::iterator i = m_sequenceLog.find(prefix);

  if (i != m_sequenceLog.end())
    {
      Seq s = i->second;
      if (s.session == session)
        return s.seq;
    }
	else
    BOOST_THROW_EXCEPTION(GetSeqException() << errmsg_info_str("No corresponding seq"));
}

bool
AppDataPublish::publishData (const string &name, uint32_t session, const string &dataBuffer, int freshness)
{
  uint32_t seq = 0;
	try
		{
			seq =  getHighestSeq(name, session);
		}
	catch (GetSeqException &e){
    m_sequenceLog.erase(name);
	}

  Seq s;
  s.session = session;
  s.seq = seq;
  m_sequenceLog[name] = s;

  ostringstream contentNameWithSeqno;
  contentNameWithSeqno << name << "/" << session << "/" << seq;

  m_ccnxHandle->publishData (contentNameWithSeqno.str (), dataBuffer, freshness);

  unordered_map<pair<string, uint32_t>, string>::iterator it = m_recentData.find(make_pair(name, session));
  if (it != m_recentData.end()) 
    m_recentData.erase(it);
  m_recentData.insert(make_pair(make_pair(name, session), dataBuffer));

  return true;
}

}
