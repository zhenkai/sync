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
 *         Chaoyi Bian <bcy@pku.edu.cn>
 *	   Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 */

#include "sync-app-socket.h"

using namespace std;
using namespace boost;

namespace Sync
{

SyncAppSocket::SyncAppSocket (const string &syncPrefix, NewDataCallback dataCallback, RemoveCallback rmCallback )
  : m_ccnxHandle (new CcnxWrapper())
  , m_newDataCallback(dataCallback)
  , m_syncLogic (syncPrefix,
                 bind(&SyncAppSocket::passCallback, this, _1),
                 rmCallback)
{
}

SyncAppSocket::~SyncAppSocket()
{
}

std::string
SyncAppSocket::GetLocalPrefix()
{
  // this handle is supposed to be short lived
  CcnxWrapperPtr handle( new CcnxWrapper());
  return handle->getLocalPrefix();
}

bool 
SyncAppSocket::publishString (const string &prefix, uint32_t session, const string &dataBuffer, int freshness)
{
  uint32_t sequence = getNextSeq(prefix, session);
  ostringstream contentNameWithSeqno;
  contentNameWithSeqno << prefix << "/" << session << "/" << sequence;
  m_ccnxHandle->publishStringData (contentNameWithSeqno.str (), dataBuffer, freshness);

  SeqNo s(session, sequence + 1);
  m_sequenceLog[prefix] = s;

  m_syncLogic.addLocalNames (prefix, session, sequence);
}

bool 
SyncAppSocket::publishRaw(const std::string &prefix, uint32_t session, const char *buf, size_t len, int freshness)
{
  uint32_t sequence = getNextSeq(prefix, session);
  ostringstream contentNameWithSeqno;
  contentNameWithSeqno << prefix << "/" << session << "/" << sequence;

  m_ccnxHandle->publishRawData (contentNameWithSeqno.str (), buf, len, freshness);

  SeqNo s(session, sequence + 1);
  m_sequenceLog[prefix] = s;
  m_syncLogic.addLocalNames (prefix, session, sequence);
}


void 
SyncAppSocket::fetchString(const std::string &prefix, const SeqNo &seq, CcnxWrapper::StringDataCallback callback, int retry)
{
  ostringstream interestName;
  interestName << prefix << "/" << seq.getSession() << "/" << seq.getSeq();
  m_ccnxHandle->sendInterestForString(interestName.str(), callback, retry);
}

void 
SyncAppSocket::fetchRaw(const std::string &prefix, const SeqNo &seq, CcnxWrapper::RawDataCallback callback, int retry)
{
  ostringstream interestName;
  interestName << prefix << "/" << seq.getSession() << "/" << seq.getSeq();
  //std::cout << "Socket " << this << " Send Interest <" << interestName.str() << "> for raw data " << endl;
  m_ccnxHandle->sendInterest(interestName.str(), callback, retry);
}

uint32_t
SyncAppSocket::getNextSeq (const string &prefix, uint32_t session)
{
  SequenceLog::iterator i = m_sequenceLog.find (prefix);

  if (i != m_sequenceLog.end ())
    {
      SeqNo s = i->second;
      if (s.getSession() == session)
        return s.getSeq();
    }
  return 0;
}

}

