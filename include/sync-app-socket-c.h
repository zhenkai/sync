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

#ifndef SYNC_APP_SOCKET_C_H
#define SYNC_APP_SOCKET_C_H

#ifdef __cplusplus
extern "C" {
#endif
  
  typedef struct SyncAppSocketStruct SyncAppSocketStruct;
  
  struct MissingDataInfoC
  {
    const char *prefix;
    int session;
    int low;
    int high;
  };

  SyncAppSocketStruct *create_sync_app_socket(const char *prefix, 
      void (*updatecallback)(const struct MissingDataInfoC*, const int, const SyncAppSocketStruct*),
      void (*removecallback)(const char *));
  void delete_sync_app_socket(SyncAppSocketStruct **sock);
  int sync_app_socket_publish(const SyncAppSocketStruct *sock, const char *prefix, int session, const char *buf, int freshness);
  void sync_app_socket_remove(const SyncAppSocketStruct *sock, const char *prefix);
  void sync_app_socket_fetch(const SyncAppSocketStruct *sock, const char *prefix, int session, int seq,
      void (*callback)(const char*, const char*), int retry);
#ifdef __cplusplus
}
#endif

#endif // SYNC_APP_SOCKET_C_H
