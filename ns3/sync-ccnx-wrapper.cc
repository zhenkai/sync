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

#include "sync-ccnx-wrapper.h"
#include "sync-log.h"
#include <boost/throw_exception.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

namespace ll = boost::lambda;

#include <ns3/ccnx-name-components.h>
#include <ns3/ccnx-interest-header.h>
#include <ns3/ccnx-content-object-header.h>
#include <ns3/ccnx-face.h>
#include <ns3/packet.h>
#include <ns3/ccnx-fib.h>

typedef boost::error_info<struct tag_errmsg, std::string> errmsg_info_str;
typedef boost::error_info<struct tag_errmsg, int> errmsg_info_int;

using namespace std;
using namespace boost;
using namespace ns3;

INIT_LOGGER ("CcnxWrapper");

namespace Sync {

CcnxWrapper::CcnxWrapper()
  : m_rand (0, std::numeric_limits<uint32_t>::max ())
{
}

CcnxWrapper::~CcnxWrapper()
{
}

void
CcnxWrapper::StartApplication ()
{
  CcnxApp::StartApplication ();
}

void
CcnxWrapper::StopApplication ()
{
  CcnxApp::StopApplication ();
}

int
CcnxWrapper::publishData (const string &dataName, const string &dataBuffer, int freshness)
{
  // NS_LOG_INFO ("Requesting Interest: \n" << interestHeader);
  _LOG_INFO ("> Data for " << dataName);

  Ptr<CcnxNameComponents> name = Create<CcnxNameComponents> ();
  istringstream is (dataName);
  is >> *name;

  static CcnxContentObjectTail trailer;
  
  CcnxContentObjectHeader data;
  data.SetName (name);
  data.SetFreshness (Seconds (freshness));

  Ptr<Packet> packet = Create<Packet> (reinterpret_cast<const uint8_t*> (dataBuffer.c_str ()), dataBuffer.size ());
  
  packet->AddHeader (data);
  packet->AddTrailer (trailer);

  m_protocolHandler (packet);

  m_transmittedContentObjects (&data, packet, this, m_face);

  return 0;
}

int CcnxWrapper::sendInterest (const string &strInterest, const DataCallback &dataCallback)
{
  // NS_LOG_INFO ("Requesting Interest: \n" << interestHeader);
  _LOG_INFO ("> Interest for " << strInterest);

  Ptr<CcnxNameComponents> name = Create<CcnxNameComponents> ();
  istringstream is (strInterest);
  is >> *name;
  
  CcnxInterestHeader interestHeader;
  interestHeader.SetNonce            (m_rand.GetValue ());
  interestHeader.SetName             (name);
  interestHeader.SetInterestLifetime (Seconds (4.0));

  Ptr<Packet> packet = Create<Packet> ();
  packet->AddHeader (interestHeader);

  // NS_LOG_DEBUG (interestHeader);
  
  m_protocolHandler (packet);

  m_transmittedInterests (&interestHeader, this, m_face);

  // Record the callback
  CcnxFilterEntryContainer<DataCallback>::type::iterator entry = m_dataCallbacks.find (*name);
  if (entry == m_dataCallbacks.end ())
    {
      pair<CcnxFilterEntryContainer<DataCallback>::type::iterator, bool> status =
        m_dataCallbacks.insert (CcnxFilterEntry<DataCallback> (name));

      entry = status.first;
    }
  m_dataCallbacks.modify (entry, ll::bind (&CcnxFilterEntry<DataCallback>::AddCallback, ll::_1, dataCallback));
  
  return 0;
}

int CcnxWrapper::setInterestFilter (const string &prefix, const InterestCallback &interestCallback)
{
  Ptr<CcnxNameComponents> name = Create<CcnxNameComponents> ();
  istringstream is (prefix);
  is >> *name;

  CcnxFilterEntryContainer<InterestCallback>::type::iterator entry = m_interestCallbacks.find (*name);
  if (entry == m_interestCallbacks.end ())
    {
      pair<CcnxFilterEntryContainer<InterestCallback>::type::iterator, bool> status =
        m_interestCallbacks.insert (CcnxFilterEntry<InterestCallback> (name));

      entry = status.first;
    }

  m_interestCallbacks.modify (entry, ll::bind (&CcnxFilterEntry<InterestCallback>::AddCallback, ll::_1, interestCallback));

  // creating actual face
  
  Ptr<CcnxFib> fib = GetNode ()->GetObject<CcnxFib> ();
  CcnxFibEntryContainer::type::iterator fibEntry = fib->Add (*name, m_face, 0);

  // make face green, so it will be used primarily
  fib->m_fib.modify (fibEntry,
                     ll::bind (&CcnxFibEntry::UpdateStatus,
                               ll::_1, m_face, CcnxFibFaceMetric::NDN_FIB_GREEN));

  return 0;
}

void
CcnxWrapper::clearInterestFilter (const std::string &prefix)
{
  Ptr<CcnxNameComponents> name = Create<CcnxNameComponents> ();
  istringstream is (prefix);
  is >> *name;

  CcnxFilterEntryContainer<InterestCallback>::type::iterator entry = m_interestCallbacks.find (*name);
  if (entry == m_interestCallbacks.end ())
    return;

  m_interestCallbacks.modify (entry, ll::bind (&CcnxFilterEntry<InterestCallback>::ClearCallback, ll::_1));  
}

CcnxFilterEntryContainer<CcnxWrapper::InterestCallback>::type::iterator
CcnxWrapper::InterestCallbackLookup (const ns3::CcnxNameComponents &name)
{
  CcnxFilterEntryContainer<InterestCallback>::type::iterator entry = m_interestCallbacks.end ();

  // do the longest prefix match
  for (size_t componentsCount = name.GetComponents ().size ()+1;
       componentsCount > 0;
       componentsCount--)
    {
      CcnxNameComponents subPrefix (name.GetSubComponents (componentsCount-1));

      entry = m_interestCallbacks.find (subPrefix);
      if (entry != m_interestCallbacks.end())
        return entry;
    }

  return entry;
}

CcnxFilterEntryContainer<CcnxWrapper::DataCallback>::type::iterator
CcnxWrapper::DataCallbackLookup (const ns3::CcnxNameComponents &name)
{
  CcnxFilterEntryContainer<DataCallback>::type::iterator entry = m_dataCallbacks.end ();

  // do the longest prefix match
  for (size_t componentsCount = name.GetComponents ().size ()+1;
       componentsCount > 0;
       componentsCount--)
    {
      CcnxNameComponents subPrefix (name.GetSubComponents (componentsCount-1));

      entry = m_dataCallbacks.find (subPrefix);
      if (entry != m_dataCallbacks.end())
        return entry;
    }

  return entry;  
}

void
CcnxWrapper::OnInterest (const Ptr<const CcnxInterestHeader> &interest, Ptr<Packet> packet)
{
  CcnxApp::OnInterest (interest, packet);

  CcnxFilterEntryContainer<InterestCallback>::type::iterator entry = InterestCallbackLookup (interest->GetName ());
  if (entry == m_interestCallbacks.end ())
    {
      _LOG_DEBUG ("No Interest callback set");
      return;
    }
  
  entry->m_callback (lexical_cast<string> (interest->GetName ()));  
}

void
CcnxWrapper::OnContentObject (const Ptr<const CcnxContentObjectHeader> &contentObject,
                              Ptr<Packet> payload)
{
  CcnxApp::OnContentObject (contentObject, payload);

  CcnxFilterEntryContainer<DataCallback>::type::iterator entry = DataCallbackLookup (contentObject->GetName ());
  if (entry == m_dataCallbacks.end ())
    {
      _LOG_DEBUG ("No Data callback set");
      return;
    }

  ostringstream content;
  payload->CopyData (&content, payload->GetSize ());
  
  entry->m_callback (lexical_cast<string> (contentObject->GetName ()), content.str ());
  
  // i guess it make sense to remove callback when interest is satisfied
  m_dataCallbacks.erase (entry);
}


}
