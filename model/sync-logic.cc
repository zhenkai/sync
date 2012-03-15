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
#include "sync-log.h"

#include <boost/make_shared.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <vector>

using namespace std;
using namespace boost;

INIT_LOGGER ("SyncLogic");

namespace Sync
{

SyncLogic::SyncLogic (const std::string &syncPrefix,
                      LogicUpdateCallback onUpdate,
                      LogicRemoveCallback onRemove)
  : m_syncPrefix (syncPrefix)
  , m_onUpdate (onUpdate)
  , m_onRemove (onRemove)
  , m_ccnxHandle(new CcnxWrapper())
  , m_randomGenerator (static_cast<unsigned int> (std::time (0)))
  , m_rangeUniformRandom (m_randomGenerator, uniform_int<> (10,50))
{
  _LOG_FUNCTION (syncPrefix);
  
  m_ccnxHandle->setInterestFilter (m_syncPrefix,
                                   bind (&SyncLogic::respondSyncInterest, this, _1));

  m_scheduler.schedule (posix_time::seconds (0),
                        bind (&SyncLogic::sendSyncInterest, this),
                        REEXPRESSING_INTEREST);
}

SyncLogic::~SyncLogic ()
{
  _LOG_FUNCTION (this);
  // cout << "SyncLogic::~SyncLogic ()" << endl;

  m_ccnxHandle.reset ();
}

void
SyncLogic::respondSyncInterest (const string &interest)
{
  _LOG_TRACE ("<< I " << interest);
  //cout << "Respond Sync Interest" << endl;
  string hash = interest.substr(interest.find_last_of("/") + 1);
  // cout << "Received Sync Interest: " << hash << endl;
  
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

  processSyncInterest (digest, interest, false);
}

void
SyncLogic::processSyncInterest (DigestConstPtr digest, const std::string &interestName, bool timedProcessing/*=false*/)
{
  recursive_mutex::scoped_lock lock (m_stateMutex);

  // Special case when state is not empty and we have received request with zero-root digest
  if (digest->isZero () && !m_state.getDigest()->isZero ())
    {
      _LOG_TRACE (">> D " << interestName << "/state" << " (zero)");

      m_ccnxHandle->publishData (interestName + "/state",
                                 lexical_cast<string> (m_state),
                                 m_syncResponseFreshness);
      return;
    }

  if (*m_state.getDigest() == *digest)
    {
      // cout << interestName << "\n";
      if (digest->isZero ())
        {
          _LOG_TRACE ("Digest is zero, adding /state to PIT");
          m_syncInterestTable.insert (interestName + "/state");
        }
      else
        {
          _LOG_TRACE ("Same state. Adding to PIT");
          m_syncInterestTable.insert (interestName);
        }
      return;
    }
  
  DiffStateContainer::iterator stateInDiffLog = m_log.find (digest);

  if (stateInDiffLog != m_log.end ())
  {
    _LOG_TRACE (">> D " << interestName);
    
    m_ccnxHandle->publishData (interestName,
                               lexical_cast<string> (*(*stateInDiffLog)->diff ()),
                               m_syncResponseFreshness);
    return;
  }

  if (!timedProcessing)
    {
      _LOG_DEBUG ("hmm");
      m_scheduler.schedule (posix_time::milliseconds (m_rangeUniformRandom ()) /*from 20 to 100ms*/,
                            bind (&SyncLogic::processSyncInterest, this, digest, interestName, true),
                            DELAYED_INTEREST_PROCESSING);
      
    }
  else
    {
      _LOG_TRACE (">> D " << interestName << "/state" << " (timed processing)");
      
      m_ccnxHandle->publishData (interestName + "/state",
                                 lexical_cast<string> (m_state),
                                 m_syncResponseFreshness);
    }
}

void
SyncLogic::processSyncData (const string &name, const string &dataBuffer)
{
  _LOG_TRACE ("<< D " << name);
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

              if (inserted || updated)
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

                  if (inserted || updated)
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
      m_log.erase (m_state.getDigest());
      /// @todo Optimization
      m_log.insert (diffLog);
    }
  catch (Error::SyncXmlDecodingFailure &e)
    {
      diffLog.reset ();
      // don't do anything
    }

  // if state has changed, then it is safe to express a new interest
  if (diffLog->getLeaves ().size () > 0)
    {
      sendSyncInterest ();
    }
  else
    {
      // should not reexpress the same interest. Need at least wait for data lifetime
      // Otherwise we will get immediate reply from the local daemon and there will be 100% utilization
      m_scheduler.cancel (REEXPRESSING_INTEREST);
      m_scheduler.schedule (posix_time::seconds (m_syncResponseFreshness),
                            bind (&SyncLogic::sendSyncInterest, this),
                            REEXPRESSING_INTEREST);
    }
}

void
SyncLogic::satisfyPendingSyncInterests (DiffStatePtr diffLog)
{
  vector<string> pis = m_syncInterestTable.fetchAll ();
  if (pis.size () > 0)
    {
      stringstream ss;
      ss << *diffLog;
      for (vector<string>::iterator ii = pis.begin(); ii != pis.end(); ++ii)
        {
          _LOG_TRACE (">> D " << *ii);
          m_ccnxHandle->publishData (*ii, ss.str(), m_syncResponseFreshness);
        }
    }
}

void
SyncLogic::processPendingSyncInterests (DiffStatePtr diffLog) 
{
  //cout << "Process Pending Interests" <<endl;
  diffLog->setDigest (m_state.getDigest());  
  if (m_log.size () > 0)
    {
      m_log.get<sequenced> ().front ()->setNext (diffLog);
    }
  m_log.erase (m_state.getDigest()); // remove diff state with the same digest.  next pointers are still valid
  /// @todo Optimization
  m_log.insert (diffLog);
  _LOG_DEBUG (*diffLog->getDigest () << " " << m_log.size ());
}

void
SyncLogic::addLocalNames (const string &prefix, uint32_t session, uint32_t seq)
{
  DiffStatePtr diff;
  {
    //cout << "Add local names" <<endl;
    recursive_mutex::scoped_lock lock (m_stateMutex);
    NameInfoConstPtr info = StdNameInfo::FindOrCreate(prefix);
  
    SeqNo seqN (session, seq);
    m_state.update(info, seqN);

    diff = make_shared<DiffState>();
    diff->update(info, seqN);
    processPendingSyncInterests (diff);
  }

  satisfyPendingSyncInterests (diff);
}

void
SyncLogic::remove(const string &prefix) 
{
  DiffStatePtr diff;
  {
    recursive_mutex::scoped_lock lock (m_stateMutex);
    NameInfoConstPtr info = StdNameInfo::FindOrCreate(prefix);
    m_state.remove(info);	

    diff = make_shared<DiffState>();
    diff->remove(info);

    processPendingSyncInterests (diff);
  }

  satisfyPendingSyncInterests (diff);
}

void
SyncLogic::sendSyncInterest ()
{
  // cout << "Sending Sync Interest" << endl;
  recursive_mutex::scoped_lock lock (m_stateMutex);

  ostringstream os;
  os << m_syncPrefix << "/" << *m_state.getDigest();

  _LOG_TRACE (">> I " << os.str ());

  m_ccnxHandle->sendInterest (os.str (),
                              bind (&SyncLogic::processSyncData, this, _1, _2));

  m_scheduler.cancel (REEXPRESSING_INTEREST);
  m_scheduler.schedule (posix_time::seconds (4),
                        bind (&SyncLogic::sendSyncInterest, this),
                        REEXPRESSING_INTEREST);
}

}
