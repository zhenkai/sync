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
#include <poll.h>
#include <boost/throw_exception.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

typedef boost::error_info<struct tag_errmsg, std::string> errmsg_info_str;
typedef boost::error_info<struct tag_errmsg, int> errmsg_info_int;

using namespace std;
using namespace boost;

INIT_LOGGER ("CcnxWrapper");

namespace Sync {

#ifdef _DEBUG_WRAPPER_      
CcnxWrapper::CcnxWrapper(char c)
#else
CcnxWrapper::CcnxWrapper()
#endif
  : m_handle (0)
  , m_keyStore (0)
  , m_keyLoactor (0)
  , m_running (true)
{
#ifdef _DEBUG_WRAPPER_      
  m_c = c;
#endif
  m_handle = ccn_create ();
  initKeyStore ();
  createKeyLocator ();
  if (ccn_connect(m_handle, NULL) < 0)
    BOOST_THROW_EXCEPTION (CcnxOperationException() << errmsg_info_str("connection to ccnd failed"));
  m_thread = thread (&CcnxWrapper::ccnLoop, this);
}

CcnxWrapper::~CcnxWrapper()
{
  // std::cout << "CcnxWrapper::~CcnxWrapper()" << std::endl;
  {
    recursive_mutex::scoped_lock lock(m_mutex);
    m_running = false;
  }
  
  m_thread.join ();
  ccn_disconnect (m_handle);
  ccn_destroy (&m_handle);
  ccn_charbuf_destroy (&m_keyLoactor);
  ccn_keystore_destroy (&m_keyStore);
}

/// @cond include_hidden

void
CcnxWrapper::createKeyLocator ()
{
  m_keyLoactor = ccn_charbuf_create();
  ccn_charbuf_append_tt (m_keyLoactor, CCN_DTAG_KeyLocator, CCN_DTAG);
  ccn_charbuf_append_tt (m_keyLoactor, CCN_DTAG_Key, CCN_DTAG);
  int res = ccn_append_pubkey_blob (m_keyLoactor, ccn_keystore_public_key(m_keyStore));
  if (res >= 0)
    {
      ccn_charbuf_append_closer (m_keyLoactor); /* </Key> */
      ccn_charbuf_append_closer (m_keyLoactor); /* </KeyLocator> */
    }
}

const ccn_pkey*
CcnxWrapper::getPrivateKey ()
{
  return ccn_keystore_private_key (m_keyStore);
}

const unsigned char*
CcnxWrapper::getPublicKeyDigest ()
{
  return ccn_keystore_public_key_digest(m_keyStore);
}

ssize_t
CcnxWrapper::getPublicKeyDigestLength ()
{
  return ccn_keystore_public_key_digest_length(m_keyStore);
}

void
CcnxWrapper::initKeyStore ()
{
  m_keyStore = ccn_keystore_create ();
  string keyStoreFile = string(getenv("HOME")) + string("/.ccnx/.ccnx_keystore");
  if (ccn_keystore_init (m_keyStore, (char *)keyStoreFile.c_str(), (char*)"Th1s1sn0t8g00dp8ssw0rd.") < 0)
    BOOST_THROW_EXCEPTION(CcnxOperationException() << errmsg_info_str(keyStoreFile.c_str()));
}

void
CcnxWrapper::ccnLoop ()
{
  _LOG_FUNCTION (this);

  while (m_running)
    {
#ifdef _DEBUG_WRAPPER_      
      std::cout << m_c << flush;
#endif
      int res = 0;
      {
        recursive_mutex::scoped_lock lock (m_mutex);
        res = ccn_run (m_handle, 0);

      }

      if (!m_running) break;
      
      if (res < 0)
        BOOST_THROW_EXCEPTION (CcnxOperationException()
                               << errmsg_info_str("ccn_run returned error"));


      pollfd pfds[1];
      {
        recursive_mutex::scoped_lock lock (m_mutex);
        
        pfds[0].fd = ccn_get_connection_fd (m_handle);
        pfds[0].events = POLLIN;
        if (ccn_output_is_pending (m_handle))
          pfds[0].events |= POLLOUT;
      }
      
      int ret = poll (pfds, 1, 1);
      if (ret < 0)
        {
          BOOST_THROW_EXCEPTION (CcnxOperationException() << errmsg_info_str("ccnd socket failed (probably ccnd got stopped)"));
        }
    }
}

/// @endcond

int
CcnxWrapper::publishData (const string &name, const string &dataBuffer, int freshness)
{
  recursive_mutex::scoped_lock lock(m_mutex);
  if (!m_running)
    return -1;
  
  // cout << "Publish: " << name << endl;
  ccn_charbuf *pname = ccn_charbuf_create();
  ccn_charbuf *signed_info = ccn_charbuf_create();
  ccn_charbuf *content = ccn_charbuf_create();

  ccn_name_from_uri(pname, name.c_str());
  ccn_signed_info_create(signed_info,
			 getPublicKeyDigest(),
			 getPublicKeyDigestLength(),
			 NULL,
			 CCN_CONTENT_DATA,
			 freshness,
			 NULL,
			 m_keyLoactor);
  if(ccn_encode_ContentObject(content, pname, signed_info,
			   dataBuffer.c_str(), dataBuffer.length (),
			   NULL, getPrivateKey()) < 0)
    BOOST_THROW_EXCEPTION(CcnxOperationException() << errmsg_info_str("encode content failed"));

  if (ccn_put(m_handle, content->buf, content->length) < 0)
    BOOST_THROW_EXCEPTION(CcnxOperationException() << errmsg_info_str("ccnput failed"));

  ccn_charbuf_destroy (&pname);
  ccn_charbuf_destroy (&signed_info);
  ccn_charbuf_destroy (&content);
  return 0;
}


static ccn_upcall_res
incomingInterest(ccn_closure *selfp,
                 ccn_upcall_kind kind,
                 ccn_upcall_info *info)
{
  CcnxWrapper::InterestCallback *f = static_cast<CcnxWrapper::InterestCallback*> (selfp->data);

  switch (kind)
    {
    case CCN_UPCALL_FINAL: // effective in unit tests
      delete f;
      delete selfp;
      return CCN_UPCALL_RESULT_OK;

    case CCN_UPCALL_INTEREST:
      break;

    default:
      return CCN_UPCALL_RESULT_OK;
    }

  string interest;
  for (int i = 0; i < info->interest_comps->n - 1; i++)
    {
      char *comp;
      size_t size;
      interest += "/";
      ccn_name_comp_get(info->interest_ccnb, info->interest_comps, i, (const unsigned char **)&comp, &size);
      string compStr(comp, size);
      interest += compStr;
    }
  (*f) (interest);
  return CCN_UPCALL_RESULT_OK;
}

static ccn_upcall_res
incomingData(ccn_closure *selfp,
             ccn_upcall_kind kind,
             ccn_upcall_info *info)
{
  //CcnxWrapper::DataCallback *f = static_cast<CcnxWrapper::DataCallback*> (selfp->data);
  ClosurePass *cp = static_cast<ClosurePass *> (selfp->data);

  switch (kind)
    {
    case CCN_UPCALL_FINAL:  // effecitve in unit tests
      delete cp;
      cp = NULL;
      delete selfp;
      return CCN_UPCALL_RESULT_OK;

    case CCN_UPCALL_CONTENT:
      break;

    case CCN_UPCALL_INTEREST_TIMED_OUT: {
      if (cp != NULL && cp->getRetry() > 0) {
        cp->decRetry();
        return CCN_UPCALL_RESULT_REEXPRESS;
      }
      return CCN_UPCALL_RESULT_OK;
    }

    default:
      return CCN_UPCALL_RESULT_OK;
    }

  char *pcontent;
  size_t len;
  if (ccn_content_get_value(info->content_ccnb, info->pco->offset[CCN_PCO_E], info->pco, (const unsigned char **)&pcontent, &len) < 0)
    BOOST_THROW_EXCEPTION(CcnxOperationException() << errmsg_info_str("decode ContentObject failed"));

  string name;
  for (int i = 0; i < info->content_comps->n - 1; i++)
    {
      char *comp;
      size_t size;
      name += "/";
      ccn_name_comp_get(info->content_ccnb, info->content_comps, i, (const unsigned char **)&comp, &size);
      string compStr(comp, size);
      name += compStr;
    }

  cp->runCallback(name, pcontent, len);

  return CCN_UPCALL_RESULT_OK;
}

int CcnxWrapper::sendInterest (const string &strInterest, const DataCallback &dataCallback, int retry)
{
  DataClosurePass * pass = new DataClosurePass(STRING_FORM, retry, dataCallback);
  sendInterest(strInterest, pass);
}

int CcnxWrapper::sendInterestForRawData (const string &strInterest, const RawDataCallback &rawDataCallback, int retry)
{
  RawDataClosurePass * pass = new RawDataClosurePass(RAW_DATA, retry, rawDataCallback);
  sendInterest(strInterest, pass);
}

int CcnxWrapper::sendInterest (const string &strInterest, void *dataPass)
{
  recursive_mutex::scoped_lock lock(m_mutex);
  if (!m_running)
    return -1;
  
  // std::cout << "Send interests for " << strInterest << std::endl;
  ccn_charbuf *pname = ccn_charbuf_create();
  ccn_closure *dataClosure = new ccn_closure;

  ccn_name_from_uri (pname, strInterest.c_str());
  dataClosure->data = dataPass;

  dataClosure->p = &incomingData;
  if (ccn_express_interest (m_handle, pname, dataClosure, NULL) < 0)
    BOOST_THROW_EXCEPTION(CcnxOperationException() << errmsg_info_str("express interest failed"));

  ccn_charbuf_destroy (&pname);
  return 0;
}

int CcnxWrapper::setInterestFilter (const string &prefix, const InterestCallback &interestCallback)
{
  recursive_mutex::scoped_lock lock(m_mutex);
  if (!m_running)
    return -1;

  ccn_charbuf *pname = ccn_charbuf_create();
  ccn_closure *interestClosure = new ccn_closure;

  ccn_name_from_uri (pname, prefix.c_str());
  interestClosure->data = new InterestCallback (interestCallback); // should be removed when closure is removed
  interestClosure->p = &incomingInterest;
  int ret = ccn_set_interest_filter (m_handle, pname, interestClosure);
  if (ret < 0)
    {
      BOOST_THROW_EXCEPTION(CcnxOperationException() << errmsg_info_str("set interest filter failed") << errmsg_info_int (ret));
    }

  ccn_charbuf_destroy(&pname);
}

void
CcnxWrapper::clearInterestFilter (const std::string &prefix)
{
  recursive_mutex::scoped_lock lock(m_mutex);
  if (!m_running)
    return;

  std::cout << "clearInterestFilter" << std::endl;
  ccn_charbuf *pname = ccn_charbuf_create();

  ccn_name_from_uri (pname, prefix.c_str());
  int ret = ccn_set_interest_filter (m_handle, pname, 0);
  if (ret < 0)
    {
      BOOST_THROW_EXCEPTION(CcnxOperationException() << errmsg_info_str("set interest filter failed") << errmsg_info_int (ret));
    }

  ccn_charbuf_destroy(&pname);
}

DataClosurePass::DataClosurePass (CallbackType type, int retry, const CcnxWrapper::DataCallback &dataCallback): ClosurePass(type, retry), m_callback(NULL)
{
   m_callback = new CcnxWrapper::DataCallback (dataCallback); 
}

DataClosurePass::~DataClosurePass () 
{
  delete m_callback;
  m_callback = NULL;
}

void 
DataClosurePass::runCallback(std::string name, const char *data, size_t len) 
{
  string content(data, len);
  if (m_callback != NULL) {
    (*m_callback)(name, content);
  }
}


RawDataClosurePass::RawDataClosurePass (CallbackType type, int retry, const CcnxWrapper::RawDataCallback &rawDataCallback): ClosurePass(type, retry), m_callback(NULL)
{
   m_callback = new CcnxWrapper::RawDataCallback (rawDataCallback); 
}

RawDataClosurePass::~RawDataClosurePass () 
{
  delete m_callback;
  m_callback = NULL;
}

void 
RawDataClosurePass::runCallback(std::string name, const char *data, size_t len) 
{
  if (m_callback != NULL) {
    (*m_callback)(name, data, len);
  }
}

}
