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

#ifndef SYNC_APP_DATA_PUBLISH_H
#define SYNC_APP_DATA_PUBLISH_H
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include "sync-ccnx-wrapper.h"

/**
 * \defgroup sync SYNC protocol
 *
 * Implementation of SYNC protocol
 */
namespace Sync {

/**
 * \ingroup sync
 * @brief publishes application data using incrementing sequence number (for
 * each sequence namber and keeps track of most recently published data for
 * each name prefix
 */
class AppDataPublish
{
public:
	AppDataPublish(boost::shared_ptr<CcnxWrapper> ccnxHandle)
	{ m_ccnxHandle = ccnxHandle; }
	~AppDataPublish() {};

	/**
	 * @brief get the name (including sequence number) and the content
	 * (unencoded, just XML stanza) of the most recent published data
	 *
	 * @param prefix the name prefix to look for
	 * @return the pair of name and content
	 */
	std::pair<std::string, std::string> getRecentData(std::string prefix);

	/**
	 * brief get the most recent sequence number for a name prefix
	 */
	 u_int32_t getHighestSeq(std::string prefix);

	/**
	 * @brief publish data for a name prefix, updates the corresponding
	 * sequence number and recent data
	 *
	 * @param name data prefix
	 * @param dataBuffer the data itself
	 * @param freshness the freshness for the data object
	 * @return whether the publish succeeded
	 */
	bool publishData(std::string name, std::string dataBuffer, int freshness);

private:
	boost::unordered_map<std::string, uint32_t> m_sequenceLog;
	boost::shared_ptr<CcnxWrapper> m_ccnxHandle;
	boost::unordered_map<std::string, std::string> m_recentData;
};

} // Sync

#endif // SYNC_APP_DATA_PUBLISH_H
