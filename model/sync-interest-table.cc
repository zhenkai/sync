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

#include "sync-interest-table.h"

using namespace std;
using namespace boost;

namespace Sync
{

unordered_set<string> SyncInterestTable::fetchAll()
{
	expireInterests();

	recursive_mutex::scoped_lock lock(m_mutex);
	unordered_set<string> entries;
	for (unordered_map<string, time_t>::iterator it = m_table.begin(); it !=
		m_table.end(); ++it) {
		entries.insert(it->first);
	}

	return entries;
}

bool SyncInterestTable::insert(string interest)
{
	recursive_mutex::scoped_lock lock(m_mutex);
	m_table.erase(m_table.find(interest));
	time_t currentTime = time(0);
	m_table.insert(make_pair(interest, currentTime));
}

SyncInterestTable::SyncInterestTable() {
	m_thread = thread(&SyncInterestTable::periodicCheck, this);
}

void SyncInterestTable::expireInterests() {
	recursive_mutex::scoped_lock lock(m_mutex);
	time_t currentTime = time(0);
	unordered_map<string, time_t>::iterator it = m_table.begin(); 
	while(it != m_table.end()) {
		time_t timestamp = it->second;
		if (currentTime - timestamp > m_checkPeriod) {
			it = m_table.erase(it);
		}
		else
			++it;
	}
}

void SyncInterestTable::periodicCheck() {
	sleep(4);
	expireInterests();
}

}
