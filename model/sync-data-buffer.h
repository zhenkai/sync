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
 * @brief handles conversion from xml encoded data ("wire_format") to states
 * data (FullState or DiffState)
 */
/**
 * \ingroup sync
 * @brief Data wrapper with size; convert from/to FullState and DiffState
 */
class DataBuffer {
private:
	unsigned char *m_buffer;
	size_t m_len;
public:
	DataBuffer() {m_buffer = NULL; m_len = 0;}
	DataBuffer(const unsigned char *buffer, size_t len);
	DataBuffer(const DataBuffer &dataBuffer);
	DataBuffer &operator=(const DataBuffer &dataBuffer);
	~DataBuffer();	
	size_t length() {return len;}
	const unsigned char *buffer() { return const_cast<const unsigned char *> (buffer); }

	DataBuffer &operator<<(FullState &fs);
	DataBuffer &operator<<(DiffState &ds);

	FullState &operator>>(FullState &fs);
	DiffState &operator>>(DiffState &ds);
};




} // Sync

#endif // SYNC_STATE_H
