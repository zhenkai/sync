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

#ifndef SYNC_APP_SOCKET_C_H
#define SYNC_APP_SOCKET_C_H

#include "sync-app-socket.h"

extern "C" {
  typedef struct SyncAppSocketStruct;

  SyncAppSocketStruct *create_sync_app_socket(const char *prefix, void (*callback)(const char *, const char *));
  void delete_sync_app_socket(SyncAppSocketStruct **sock);
  bool sync_app_socket_publish(SyncAppSocketStruct *sock, const char *prefix, uint32_t session, const char *buf, int freshness);
  void sync_app_socket_remove(SyncAppSocketStruct *sock, const char *prefix);
}

#endif // SYNC_APP_SOCKET_C_H
