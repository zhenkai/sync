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

#ifndef _STANDALONE
INIT_LOGGER ("SyncLogic");
#endif


#ifdef NS3_MODULE
#define GET_RANDOM(var) var.GetValue ()
#else
#define GET_RANDOM(var) var ()
#endif

#define TIME_SECONDS_WITH_JITTER(sec) \
  (TIME_SECONDS (sec) + TIME_MILLISECONDS (GET_RANDOM (m_reexpressionJitter)))

#define TIME_MILLISECONDS_WITH_JITTER(sec) \
  (TIME_MILLISECONDS (sec) + TIME_MILLISECONDS (GET_RANDOM (m_reexpressionJitter)))


namespace Sync
{

SyncLogic::SyncLogic (const std::string &syncPrefix,
                      LogicUpdateCallback onUpdate,
                      LogicRemoveCallback onRemove)
  : m_syncPrefix (syncPrefix)
  , m_onUpdate (onUpdate)
  , m_onRemove (onRemove)
  , m_ccnxHandle(new CcnxWrapper())
#ifndef NS3_MODULE
  , m_randomGenerator (static_cast<unsigned int> (std::time (0)))
  , m_rangeUniformRandom (m_randomGenerator, uniform_int<> (200,300))
  , m_reexpressionJitter (m_randomGenerator, uniform_int<> (0,100))
#else
  , m_rangeUniformRandom (200,300)
  , m_reexpressionJitter (0, 100)
#endif
{
#ifdef _STANDALONE
#ifdef _DEBUG
#ifdef HAVE_LOG4CXX
  // _LOG_FUNCTION (syncPrefix);
  static int id = 0;
  staticModuleLogger = log4cxx::Logger::getLogger ("SyncLogic." + lexical_cast<string> (id));
  id ++;
#endif
#endif
#endif

  _LOG_DEBUG ("syncPrefix");
  
#ifndef NS3_MODULE
  // In NS3 module these functions are moved to StartApplication method
  
  m_ccnxHandle->setInterestFilter (m_syncPrefix,
                                   bind (&SyncLogic::respondSyncInterest, this, _1));

  m_scheduler.schedule (TIME_SECONDS (0), // no need to add jitter
                        bind (&SyncLogic::sendSyncInterest, this),
                        REEXPRESSING_INTEREST);
#endif
}

SyncLogic::~SyncLogic ()
{
  // _LOG_FUNCTION (this);
  // cout << "SyncLogic::~SyncLogic ()" << endl;

  m_ccnxHandle.reset ();
}

#ifdef NS3_MODULE
void
SyncLogic::StartApplication ()
{
  m_ccnxHandle->SetNode (GetNode ());
  m_ccnxHandle->StartApplication ();

  m_ccnxHandle->setInterestFilter (m_syncPrefix,
                                   bind (&SyncLogic::respondSyncInterest, this, _1));

  m_scheduler.schedule (TIME_SECONDS (0), // need to send first interests at exactly the same time
                        bind (&SyncLogic::sendSyncInterest, this),
                        REEXPRESSING_INTEREST);
}

void
SyncLogic::StopApplication ()
{
  m_ccnxHandle->clearInterestFilter (m_syncPrefix);
  m_ccnxHandle->StopApplication ();
  m_scheduler.cancel (REEXPRESSING_INTEREST);
  m_scheduler.cancel (DELAYED_INTEREST_PROCESSING);
}
#endif


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

      m_syncInterestTable.remove (interestName + "/state");
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
          _LOG_TRACE ("processSyncInterest (): Digest is zero, adding /state to PIT");
          m_syncInterestTable.insert (interestName + "/state");
        }
      else
        {
          _LOG_TRACE ("processSyncInterest (): Same state. Adding to PIT");
          m_syncInterestTable.insert (interestName);
        }
      
      // !!! important change !!!
      m_scheduler.cancel (REEXPRESSING_INTEREST);
      m_scheduler.schedule (TIME_SECONDS_WITH_JITTER (m_syncInterestReexpress),
                            bind (&SyncLogic::sendSyncInterest, this),
                            REEXPRESSING_INTEREST);
      return;
    }
  
  DiffStateContainer::iterator stateInDiffLog = m_log.find (digest);

  if (stateInDiffLog != m_log.end ())
  {
    DiffStateConstPtr stateDiff = (*stateInDiffLog)->diff ();
    // string state = lexical_cast<string> (*stateDiff);
    // erase_all (state, "\n");
    // _LOG_TRACE (">> D " << interestName << ", state: " << state);
    // _LOG_DEBUG ("Log size: " << m_log.size ());

    // BOOST_FOREACH (DiffStateConstPtr ds, m_log.get<sequenced> ())
    //   {
    //     string state = lexical_cast<string> (*ds);
    //     erase_all (state, "\n");
    //     _LOG_DEBUG ("   " << state << ", " << *ds->getDigest ());
    //   }

    m_syncInterestTable.remove (interestName);
    _LOG_DEBUG (">> D" << interestName);
    m_ccnxHandle->publishData (interestName,
                               lexical_cast<string> (*stateDiff),
                               m_syncResponseFreshness);
    if (m_outstandingInterest == interestName)
      {
        m_scheduler.cancel (REEXPRESSING_INTEREST);
        m_scheduler.schedule (TIME_SECONDS_WITH_JITTER (0),
                              bind (&SyncLogic::sendSyncInterest, this),
                              REEXPRESSING_INTEREST);
      }
    return;
  }

  if (!timedProcessing)
    {
      UnknownDigestContainer::index<hashed>::type::iterator previousUnknownDigest = m_recentUnknownDigests.get<hashed> ().find (digest);
      if (previousUnknownDigest != m_recentUnknownDigests.get<hashed> ().end ())
        {
          _LOG_DEBUG ("Digest is not in the log, but we have already seen it.                      (Don't do anything)");
        }
      else
        {
          m_recentUnknownDigests.insert (DigestTime (digest, TIME_NOW + TIME_SECONDS (m_unknownDigestStoreTime)));
          
          uint32_t waitDelay = GET_RANDOM (m_rangeUniformRandom);      
          _LOG_DEBUG ("Digest is not in the log. Schedule processing after small delay: " << waitDelay << "ms");

          m_scheduler.schedule (TIME_MILLISECONDS (waitDelay),
                                bind (&SyncLogic::processSyncInterest, this, digest, interestName, true),
                                DELAYED_INTEREST_PROCESSING);
        }
    }
  else
    {
      _LOG_TRACE (">> D " << interestName << "/state" << " (timed processing)");

      m_syncInterestTable.remove (interestName + "/state");
      m_ccnxHandle->publishData (interestName + "/state",
                                 lexical_cast<string> (m_state),
                                 m_syncResponseFreshness);

      if (m_outstandingInterest == interestName)
        {
          m_scheduler.cancel (REEXPRESSING_INTEREST);
          m_scheduler.schedule (TIME_SECONDS_WITH_JITTER (0),
                                bind (&SyncLogic::sendSyncInterest, this),
                                REEXPRESSING_INTEREST);
        }
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

      m_syncInterestTable.remove (name); // just in case, remove both interests from our interest table. No reason to keep them there
      if (last == "state")
        {
          m_syncInterestTable.remove (name.substr(0, name.find_last_of("/")));
          
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
                  m_onUpdate (info->toString (), seq, oldSeq);
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
                      m_onUpdate (info->toString (), seq, oldSeq);
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

      insertToDiffLog (diffLog);
    }
  catch (Error::SyncXmlDecodingFailure &e)
    {
      diffLog.reset ();
      // don't do anything
    }

  // if state has changed, then it is safe to express a new interest
  if (diffLog->getLeaves ().size () > 0)
    {
      m_scheduler.cancel (REEXPRESSING_INTEREST);
      m_scheduler.schedule (TIME_SECONDS_WITH_JITTER (0),
                            bind (&SyncLogic::sendSyncInterest, this),
                            REEXPRESSING_INTEREST);
    }
  else
    {
      // should not reexpress the same interest. Need at least wait for data lifetime
      // Otherwise we will get immediate reply from the local daemon and there will be 100% utilization
      m_scheduler.cancel (REEXPRESSING_INTEREST);
      m_scheduler.schedule (TIME_MILLISECONDS_WITH_JITTER (m_syncResponseFreshness),
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
      bool satisfiedOwnInterest = false;
      
      for (vector<string>::iterator ii = pis.begin(); ii != pis.end(); ++ii)
        {
          _LOG_TRACE (">> D " << *ii);
          m_ccnxHandle->publishData (*ii, ss.str(), m_syncResponseFreshness);

          {
            recursive_mutex::scoped_lock lock (m_stateMutex);
            // _LOG_DEBUG (*ii << " == " << m_outstandingInterest << " = " << (*ii == m_outstandingInterest));
            satisfiedOwnInterest = satisfiedOwnInterest || (*ii == m_outstandingInterest) || (*ii == (m_outstandingInterest + "/state"));
          }
        }

      if (satisfiedOwnInterest)
        {
          _LOG_DEBUG ("Have satisfied our own interest. Scheduling interest reexpression");
          // we need to reexpress interest only if we satisfied our own interest
          m_scheduler.cancel (REEXPRESSING_INTEREST);
          m_scheduler.schedule (TIME_SECONDS_WITH_JITTER (0),
                                bind (&SyncLogic::sendSyncInterest, this),
                                REEXPRESSING_INTEREST);
        }
    }
}

void
SyncLogic::insertToDiffLog (DiffStatePtr diffLog) 
{
  //cout << "Process Pending Interests" <<endl;
  diffLog->setDigest (m_state.getDigest());  
  if (m_log.size () > 0)
    {
      m_log.get<sequenced> ().front ()->setNext (diffLog);
    }
  m_log.erase (m_state.getDigest()); // remove diff state with the same digest.  next pointers are still valid
  /// @todo Optimization
  m_log.get<sequenced> ().push_front (diffLog);
  // _LOG_DEBUG (*diffLog->getDigest () << " " << m_log.size ());
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

    _LOG_DEBUG ("addLocalNames (): new state " << *m_state.getDigest ());
    
    diff = make_shared<DiffState>();
    diff->update(info, seqN);
    insertToDiffLog (diff);
  }

  // _LOG_DEBUG ("PIT size: " << m_syncInterestTable.size ());
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

    insertToDiffLog (diff);
  }

  satisfyPendingSyncInterests (diff);  
}

void
SyncLogic::sendSyncInterest ()
{
  ostringstream os;

  {
    // cout << "Sending Sync Interest" << endl;
    recursive_mutex::scoped_lock lock (m_stateMutex);

    os << m_syncPrefix << "/" << *m_state.getDigest();

    _LOG_TRACE (">> I " << os.str ());

    m_outstandingInterest = os.str ();
  }
  
  m_ccnxHandle->sendInterest (os.str (),
                              bind (&SyncLogic::processSyncData, this, _1, _2));

  m_scheduler.cancel (REEXPRESSING_INTEREST);
  m_scheduler.schedule (TIME_SECONDS_WITH_JITTER (m_syncInterestReexpress),
                        bind (&SyncLogic::sendSyncInterest, this),
                        REEXPRESSING_INTEREST);
}

}
