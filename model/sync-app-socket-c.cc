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

#include "sync-app-socket.h"
using namespace std;
using namespace Sync;

class CallbackHolder{
private:
  void (*m_callback)(const char *, const char *);

public:
  CallbackHolder(void (*callback)(const char*, const char*)):m_callback(callback){};
  void callbackWrapper(string name, string data) {
    m_callback(name.c_str(), data.c_str());
  }
};

struct SyncAppSocketStruct;

extern "C"
SyncAppSocketStruct *
create_sync_app_socket(const char *prefix, void (*callback)(const char *, const char *)) 
{
  CallbackHolder holder(callback);
  boost::function<void (string, string)> cb = bind(&CallbackHolder::callbackWrapper, &holder, _1, _2);
  SyncAppSocket *sock = new SyncAppSocket(prefix, cb);
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
sync_app_socket_publish(SyncAppSocketStruct *sock, const char *prefix, int session, const char *buf, int freshness) 
{
  SyncAppSocket *temp = (SyncAppSocket *)sock;
  return temp->publish(prefix, session, buf, freshness);
}

extern "C"
void
sync_app_socket_remove(SyncAppSocketStruct *sock, const char *prefix) 
{
  SyncAppSocket *temp = (SyncAppSocket *)sock;
  temp->remove(prefix);
}
