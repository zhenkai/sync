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

#ifndef SYNC_LEAF_H
#define SYNC_LEAF_H
#include <boost/function.hpp>

namespace Sync {

/**
 * \ingroup sync
 * @brief A simple interface to interact with client code
 */
class SyncAppSocket
{
public:
	SyncAppSocket(std::string syncPrefix, boost::function<void
	(boost::shared_ptr<DataBuffer>)>);
	~SyncAppSocket();
	bool publishData(std::string prefix, boost::shared_ptr<DataBuffer>
	dataBuffer);
private:
	boost::shared_ptr<AppDataFetch> m_fetcher;
	boost::shared_ptr<AppDataPublish> m_publisher;
	boost::shared_ptr<SyncAppWrapper> m_syncAppWrapper;
};

} // Sync

#endif // SYNC_LEAF_H
