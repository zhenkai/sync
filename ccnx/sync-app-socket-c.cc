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
#include <boost/shared_array.hpp>
using namespace std;
using namespace Sync;

struct SyncAppSocketStruct;
struct MissingDataInfoC
{
  const char *prefix;
  int session;
  int low;
  int high;
};

class CallbackHolder {
private:
  void (*m_callback)(const char *, const char *);

public:
  CallbackHolder(void (*callback)(const char*, const char*))
    :m_callback(callback){};
  
  void callbackWrapper(const string &name, const string &data) {
    if (m_callback != NULL)
      (*m_callback)(name.c_str(), data.c_str());
  }
};

class UpdateCallbackHolder {
private:
  void (*m_callback)(const struct MissingDataInfoC*, const int, const SyncAppSocketStruct*);

public:
  UpdateCallbackHolder(void (*callback)(
        const MissingDataInfoC*, 
        const int, 
        const SyncAppSocketStruct*))
    :m_callback(callback){};

  void callbackWrapper(const vector<MissingDataInfo> &mdi, SyncAppSocket* socket) {
    boost::shared_array<MissingDataInfoC> mdic(new MissingDataInfoC[mdi.size()]);
    int i;
    
    for (i = 0; i < mdi.size(); i++)
    {
      mdic[i].prefix = mdi[i].prefix.c_str();
      mdic[i].session = mdi[i].high.getSession();
      if (mdi[i].low.getSession() != mdi[i].high.getSession())
        mdic[i].low = 0;
      else
        mdic[i].low = mdi[i].low.getSeq();
      mdic[i].high = mdi[i].high.getSeq();
    }
    if (m_callback != NULL)
      (*m_callback)(mdic.get(), i, (const SyncAppSocketStruct*) socket);
  }
};

class RemoveCallbackHolder {
private:
  void (*m_callback)(const char*);

public:
  RemoveCallbackHolder(void (*callback)(const char*)):m_callback(callback){};
  void callbackWrapper(const string &prefix) {
    if (m_callback != NULL)
      (*m_callback)(prefix.c_str());
  }
};

extern "C"
SyncAppSocketStruct *
create_sync_app_socket(const char *prefix, 
    void (*updatecallback)(const MissingDataInfoC*, const int, const SyncAppSocketStruct*),
    void (*removecallback)(const char *))
{
  boost::shared_ptr<UpdateCallbackHolder> uh(new UpdateCallbackHolder(updatecallback));
  boost::shared_ptr<RemoveCallbackHolder> rh(new RemoveCallbackHolder(removecallback));
  boost::function<void (const vector<MissingDataInfo>&, SyncAppSocket*)> ucb = bind(&UpdateCallbackHolder::callbackWrapper, uh, _1, _2);
  boost::function<void (const string&)> rcb = bind(&RemoveCallbackHolder::callbackWrapper, rh, _1);
  SyncAppSocket *sock = new SyncAppSocket(prefix, ucb, rcb);
  return (SyncAppSocketStruct *) sock;
}

extern "C"
void
delete_sync_app_socket(SyncAppSocketStruct **sock) {
  SyncAppSocket *temp = *((SyncAppSocket **)sock);
  delete temp;
  temp = NULL;
}

// assume char *buf ends with '\0', or otherwise it's going to crash;
// should fix this "feature"
extern "C" 
int
sync_app_socket_publish(const SyncAppSocketStruct *sock, const char *prefix, int session, const char *buf, int freshness) 
{
  SyncAppSocket *temp = (SyncAppSocket*) sock;
  return temp->publishString(prefix, session, buf, freshness);
}

extern "C"
void
sync_app_socket_remove(const SyncAppSocketStruct *sock, const char *prefix) 
{
  SyncAppSocket *temp = (SyncAppSocket*) sock;
  temp->remove(prefix);
}

extern "C"
void
sync_app_socket_fetch(const SyncAppSocketStruct *sock, const char *prefix, int session, int sequence, 
    void (*callback)(const char*, const char*), int retry)
{
  SyncAppSocket *temp = (SyncAppSocket*) sock;
  string s(prefix);
  SeqNo seq(session, sequence);
  boost::shared_ptr<CallbackHolder> h(new CallbackHolder(callback));
  boost::function<void (const string&, const string&)> cb = bind(&CallbackHolder::callbackWrapper, h, _1, _2);

  temp->fetchString(s, seq, cb, retry);
}

