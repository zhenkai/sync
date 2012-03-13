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
#include "sync-diff-leaf.h"
#include "sync-full-leaf.h"
#include <boost/make_shared.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <vector>

using namespace std;
using namespace boost;

namespace Sync
{

SyncLogic::SyncLogic (const std::string &syncPrefix,
                      LogicUpdateCallback onUpdate,
                      LogicRemoveCallback onRemove,
                      CcnxWrapperPtr ccnxHandle)
  : m_syncPrefix (syncPrefix)
  , m_onUpdate (onUpdate)
  , m_onRemove (onRemove)
  , m_ccnxHandle (ccnxHandle)
  , m_randomGenerator (static_cast<unsigned int> (std::time (0)))
  , m_rangeUniformRandom (m_randomGenerator, uniform_int<> (20,100))
{
  m_ccnxHandle->setInterestFilter (syncPrefix,
                                   bind (&SyncLogic::respondSyncInterest, this, _1));


}

void SyncLogic::start()
{
	// We need to send out our Sync Interest when we're ready
	sendSyncInterest();
}

SyncLogic::~SyncLogic ()
{
}


void
SyncLogic::respondSyncInterest (const string &interest)
{
	cout << "Respond Sync Interest" << endl;
  string hash = interest.substr(interest.find_last_of("/") + 1);
  DigestPtr digest = make_shared<Digest> ();
  try
    {
      istringstream is (hash);
      is >> *digest;
    }
  catch (Error::DigestCalculationError &e)
    {
      // log error. ignoring it for now, later we should log it
      return;
    }

  processSyncInterest (digest, interest);
}

void
SyncLogic::processSyncInterest (DigestConstPtr digest, const std::string &interestName, bool timedProcessing/*=false*/)
{
  //cout << "SyncLogic::processSyncInterest " << timedProcessing << endl;
  recursive_mutex::scoped_lock lock (m_stateMutex);
    
  if (*m_state.getDigest() == *digest)
  {
    m_syncInterestTable.insert (interestName);
    return;
  }

  DiffStateContainer::iterator stateInDiffLog = m_log.find (digest);

  if (stateInDiffLog != m_log.end ())
  {
    m_ccnxHandle->publishData (interestName,
                               lexical_cast<string> (*(*stateInDiffLog)->diff ()),
                               m_syncResponseFreshness);
    return;
  }

  if (!timedProcessing)
    {
      m_delayedChecksScheduler.schedule (posix_time::milliseconds (m_rangeUniformRandom ()) /*from 20 to 100ms*/,
                                         bind (&SyncLogic::processSyncInterest, this, digest, interestName, true));
      
    }
  else
    {
      m_ccnxHandle->publishData (interestName + "/state",
                                 lexical_cast<string> (m_state),
                                 m_syncResponseFreshness);
    }
}

void
SyncLogic::processSyncData (const string &name, const string &dataBuffer)
{
	cout << "Process Sync Data" <<endl;
  DiffStatePtr diffLog = make_shared<DiffState> ();
  
  try
    {
      recursive_mutex::scoped_lock lock (m_stateMutex);
      
      string last = name.substr(name.find_last_of("/") + 1);
      istringstream ss (dataBuffer);

      if (last == "state")
        {
          FullState full;
          ss >> full;
          BOOST_FOREACH (LeafConstPtr leaf, full.getLeaves()) // order doesn't matter
            {
              NameInfoConstPtr info = leaf->getInfo ();
              SeqNo seq = leaf->getSeq ();

              bool inserted = false;
              bool updated = false;
              SeqNo oldSeq;
              tie (inserted, updated, oldSeq) = m_state.update (info, seq);

              if (updated)
                {
                  diffLog->update (info, seq);
                  m_onUpdate (info->toString (), seq.getSeq(), oldSeq);
                }
            }
        }
      else
        {
          DiffState diff;
          ss >> diff;
          BOOST_FOREACH (LeafConstPtr leaf, diff.getLeaves().get<ordered>())
            {
              DiffLeafConstPtr diffLeaf = dynamic_pointer_cast<const DiffLeaf> (leaf);
              BOOST_ASSERT (diffLeaf != 0);

              NameInfoConstPtr info = diffLeaf->getInfo();
              if (diffLeaf->getOperation() == UPDATE)
                {
                  SeqNo seq = diffLeaf->getSeq();

                  bool inserted = false;
                  bool updated = false;
                  SeqNo oldSeq;
                  tie (inserted, updated, oldSeq) = m_state.update (info, seq);

                  if (updated)
                    {
                      diffLog->update (info, seq);
                      m_onUpdate (info->toString (), seq.getSeq(), oldSeq);
                    }
                }
              else if (diffLeaf->getOperation() == REMOVE)
                {
                  if (m_state.remove (info))
                    {
                      diffLog->remove (info);
                      m_onRemove (info->toString ());
                    }
                }
              else
                {
                  BOOST_ASSERT (false); // just in case
                }
            }
        }

      diffLog->setDigest(m_state.getDigest());
      if (m_log.size () > 0)
        {
          m_log.get<sequenced> ().front ()->setNext (diffLog);
        }
      m_log.insert (diffLog);
    }
  catch (Error::SyncXmlDecodingFailure &e)
    {
      // log error
      return;
    }

	// Zhenkai: shouldn't we all ways send a new Sync Interest?
  //if (diffLog->getLeaves ().size () > 0)
    {
      sendSyncInterest();
    }
}

void
SyncLogic::processPendingSyncInterests(DiffStatePtr &diff) 
{
	//cout << "Process Pending Interests" <<endl;
  recursive_mutex::scoped_lock lock (m_stateMutex);
  diff->setDigest(m_state.getDigest());
  m_log.insert(diff);

  vector<string> pis = m_syncInterestTable.fetchAll ();
  stringstream ss;
  ss << *diff;
  for (vector<string>::iterator ii = pis.begin(); ii != pis.end(); ++ii)
  {
    m_ccnxHandle->publishData (*ii, ss.str(), m_syncResponseFreshness);
  }
}

void
SyncLogic::addLocalNames (const string &prefix, uint32_t session, uint32_t seq)
{
	//cout << "Add local names" <<endl;
  recursive_mutex::scoped_lock lock (m_stateMutex);
  NameInfoConstPtr info = StdNameInfo::FindOrCreate(prefix);
  SeqNo seqN(session, seq);
  m_state.update(info, seqN);

  DiffStatePtr diff = make_shared<DiffState>();
  diff->update(info, seqN);

  processPendingSyncInterests(diff);
}

void
SyncLogic::remove(const string &prefix) 
{
  recursive_mutex::scoped_lock lock (m_stateMutex);
  NameInfoConstPtr info = StdNameInfo::FindOrCreate(prefix);
  m_state.remove(info);	

  DiffStatePtr diff = make_shared<DiffState>();
  diff->remove(info);

  processPendingSyncInterests(diff);
}

void
SyncLogic::sendSyncInterest ()
{
	cout << "Sending Sync Interest" << endl;
  recursive_mutex::scoped_lock lock (m_stateMutex);

  ostringstream os;
  os << m_syncPrefix << "/" << m_state.getDigest();

  m_ccnxHandle->sendInterest (os.str (),
                              bind (&SyncLogic::processSyncData, this, _1, _2));
}

}
