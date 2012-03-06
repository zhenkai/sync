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

#ifndef SYNC_STATE_H
#define SYNC_STATE_H

#include <tinyxml.h>
#include <boost/shared_ptr.hpp>
#include "sync-diff-state.h"
#include "sync-full-state.h"


/**
 * \defgroup sync SYNC protocol
 *
 * Implementation of SYNC protocol
 */
namespace Sync {

/**
 * \ingroup sync
 * @brief DataBuffer Interface
 */
class DataBuffer {
public:
	virtual ~DataBuffer() = 0;
	virtual size_t length() = 0; 
	virtual const unsigned char *buffer() = 0;
	virtual void setBufferAndLength(const unsigned char *buffer, size_t len) =
	0;
};

/**
 * \ingroup sync
 * @brief general Data Buffer class, mainly works for app data
 */
class AppDataBuffer:DataBuffer {
public:
	AppDataBuffer() {m_buffer = NULL; m_len = 0;}
	AppDataBuffer(const unsigned char *buffer, size_t len);
	AppDataBuffer(const DataBuffer *DataBuffer);
	AppDataBuffer &operator=(const DataBuffer *DataBuffer);
	virtual void setBufferAndLength(const unsigned char *buffer, size_t len);
	virtual ~DataBuffer();
	virtual size_t length() {return len;}
	virtual const unsigned char *buffer() { return const_cast<const unsigned char *> (buffer); }

private:
	unsigned char *m_buffer;
	size_t m_len;
};



/**
 * \ingroup sync
 * @brief decorator class, wrapper for sync data; converts to and from states
 */
class SyncDataBuffer : DataBuffer{
public:
	SyncDataBuffer(DataBuffer *dataBuffer) { m_dataBuffer = dataBuffer;}
	virtual ~SyncDataBuffer(){};	
	virtual size_t length() {m_dataBuffer->length();}
	virtual const unsigned char *buffer() {m_dataBuffer->buffer();}
	virtual void setBufferAndLength(const unsigned char *buffer, size_t len)
	{m_dataBuffer->setBufferAndLength(buffer, len); }

	SyncDataBuffer &operator<<(FullState &fs);
	SyncDataBuffer &operator<<(DiffState &ds);

	FullState &operator>>(FullState &fs);
	DiffState &operator>>(DiffState &ds);

private:
	boost::shared_ptr<DataBuffer> m_dataBuffer;
};

} // Sync

#endif // SYNC_STATE_H
