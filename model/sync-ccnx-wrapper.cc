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

#include "sync-ccnx-wrapper.h"
#include <poll.h>

using namespace std;
using namespace boost;

namespace Sync {

CcnxWrapper::CcnxWrapper()
{
  m_handle = ccn_create();
  ccn_connect(m_handle, NULL);
  initKeyStore();
  createKeyLocator();
  m_thread = thread(&CcnxWrapper::ccnLoop, this);
}

CcnxWrapper::~CcnxWrapper()
{
  running = false;
  m_thread.join();
  ccn_disconnect(m_handle);
  ccn_destroy(&m_handle);
  ccn_charbuf_destroy(&m_keyLoactor);
  ccn_keystore_destroy(&m_keyStore);
}

void CcnxWrapper::createKeyLocator()
{
  int res;

  m_keyLoactor = ccn_charbuf_create();
  ccn_charbuf_append_tt(m_keyLoactor, CCN_DTAG_KeyLocator, CCN_DTAG);
  ccn_charbuf_append_tt(m_keyLoactor, CCN_DTAG_Key, CCN_DTAG);
  res = ccn_append_pubkey_blob(m_keyLoactor, ccn_keystore_public_key(m_keyStore));
  if (res >= 0)
  {
    ccn_charbuf_append_closer(m_keyLoactor); /* </Key> */
    ccn_charbuf_append_closer(m_keyLoactor); /* </KeyLocator> */
  }
}

const ccn_pkey* CcnxWrapper::getPrivateKey()
{
  return ccn_keystore_private_key(m_keyStore);
}

const unsigned char* CcnxWrapper::getPublicKeyDigest()
{
  return ccn_keystore_public_key_digest(m_keyStore);
}

ssize_t CcnxWrapper::getPublicKeyDigestLength()
{
  return ccn_keystore_public_key_digest_length(m_keyStore);
}

void CcnxWrapper::initKeyStore()
{
  ccn_charbuf *temp = ccn_charbuf_create();
  m_keyStore = ccn_keystore_create();
  ccn_charbuf_putf(temp, "%s/.ccnx/.ccnx_keystore", getenv("HOME"));
  ccn_keystore_init(m_keyStore, ccn_charbuf_as_string(temp), (char*)"Th1s1sn0t8g00dp8ssw0rd.");
  ccn_charbuf_destroy(&temp);
}

void CcnxWrapper::ccnLoop()
{
  pollfd pfds[1];
  int res = ccn_run(m_handle, 0);

  pfds[0].fd = ccn_get_connection_fd(m_handle);
  pfds[0].events = POLLIN;

  while (running)
  {
    if (res >= 0)
    {
      int ret = poll(pfds, 1, 100);
      if (ret >= 0)
      {
	boost::recursive_mutex::scoped_lock lock(m_mutex);
	res = ccn_run(m_handle, 0);
      }
    }
  }
}

int CcnxWrapper::publishData(string name, string dataBuffer, int freshness)
{
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
  ccn_encode_ContentObject(content, pname, signed_info,
			   dataBuffer.c_str(), dataBuffer.length(),
			   NULL, getPrivateKey());
  ccn_put(m_handle, content->buf, content->length);

  ccn_charbuf_destroy(&pname);
  ccn_charbuf_destroy(&signed_info);
  ccn_charbuf_destroy(&content);
}


static ccn_upcall_res incomingInterest(
  ccn_closure *selfp,
  ccn_upcall_kind kind,
  ccn_upcall_info *info)
{
  function<void (string)> f = *(function<void (string)> *)selfp->data;

  switch (kind)
  {
    case CCN_UPCALL_FINAL:
      free(selfp);
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
    interest += comp;
  }
  f(interest);
  return CCN_UPCALL_RESULT_OK;
}

static ccn_upcall_res incomingData(
  ccn_closure *selfp,
  ccn_upcall_kind kind,
  ccn_upcall_info *info)
{
  function<void (string)> f = *(function<void (string)> *)selfp->data;

  switch (kind)
  {
    case CCN_UPCALL_FINAL:
      free(selfp);
      return CCN_UPCALL_RESULT_OK;

    case CCN_UPCALL_CONTENT:
      break;

    default:
      return CCN_UPCALL_RESULT_OK;
  }

  char *pcontent;
  size_t len;
  ccn_content_get_value(info->content_ccnb, info->pco->offset[CCN_PCO_E], info->pco, (const unsigned char **)&pcontent, &len);
  f((string)pcontent);
}

int CcnxWrapper::sendInterest(string strInterest, function<void (string)> dataCallback)
{
  ccn_charbuf *pname = ccn_charbuf_create();
  ccn_closure *dataClosure = new ccn_closure;
  function<void (string)> *f = new function<void (string)>(dataCallback);

  ccn_name_from_uri(pname, strInterest.c_str());
  ccn_express_interest(m_handle, pname, dataClosure, NULL);
  dataClosure->data = f;
  dataClosure->p = &incomingData;

  ccn_charbuf_destroy(&pname);
}

int CcnxWrapper::setInterestFilter(string prefix, function<void (string)> interestCallback)
{
  ccn_charbuf *pname = ccn_charbuf_create();
  ccn_closure *interestClosure = new ccn_closure;
  function<void (string)> *f = new function<void (string)>(interestCallback);

  ccn_name_from_uri(pname, prefix.c_str());
  interestClosure->data = f;
  interestClosure->p = &incomingInterest;
  ccn_set_interest_filter(m_handle, pname, interestClosure);

  ccn_charbuf_destroy(&pname);
}

}
