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
#include <string>
#include <boost/unordered_set.hpp>

/**
 * \defgroup sync SYNC protocol
 *
 * Implementation of SYNC protocol
 */
namespace Sync {

/**
 * \ingroup sync
 * @brief A table to keep unanswered Sync Interest
 */
class SyncInterestTable
{
private:
	boost::unordered_set<std::string> m_table;
public:
	/**
	 * @brief Insert an interest
	 */
	bool insert(std::string interest);

	/**
	 * @brief fetch all Interests and clear the table
	 */
	boost::unordered_set<std::string> fetchAll();

};

} // Sync

#endif // SYNC_STATE_H
