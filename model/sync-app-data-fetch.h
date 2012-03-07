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

#ifndef SYNC_APP_DATA_FETCH_H
#define SYNC_APP_DATA_FETCH_H
#include "sync-ccnx-wrapper.h"
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

namespace Sync {

/**
 * \ingroup sync
 * @brief fetch application data, by default it will try to fetch every piece
 * of data
 */
class AppDataFetch
{
public:
	/**
	 * @param dataCallback the callback function to process data
	 */
	AppDataFetch(boost::function<void (boost::shared_ptr<DataBuffer>)>
	dataCallback);

	void setDataCallback(boost::function<void (boost::shared_ptr<DataBuffer>)>
	dataCallback) {m_dataCallback = dataCallback;}

	/**
	 * @brief fetch data for a certain name prefix
	 *
	 * @param prefix the prefix for the data
	 * @param startSeq the start of sequence number range (inclusive)
	 * @param endSeq the end of sequence number range (inclusive)
	 */
	void fetch(string prefix, long startSeq, long endSeq);

private:
	boost::shared_ptr<CcnxWrapper> ccnxHandle;
	boost::shared_ptr<boost::function<void (boost::shared_ptr<DataBuffer>)>
	m_dataCallback;
};


} // Sync

#endif // SYNC_APP_DATA_FETCH_H
