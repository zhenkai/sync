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

SyncInterestTable::SyncInterestTable ()
  : m_running (true)
{
  m_thread = thread (&SyncInterestTable::periodicCheck, this);
}

SyncInterestTable::~SyncInterestTable ()
{
  // cout << "request interrupt: " << this_thread::get_id () << endl;
  m_running = false;
  m_thread.interrupt ();
  m_thread.join ();
}

vector<string>
SyncInterestTable::fetchAll ()
{
  expireInterests();

  recursive_mutex::scoped_lock lock (m_mutex);
  
  vector<string> entries;
  for (unordered_map<string, time_t>::iterator it = m_table.begin();
       it != m_table.end();
       ++it)
    {
      entries.push_back(it->first);
    }
  m_table.clear ();

  return entries;
}

bool
SyncInterestTable::insert(string interest)
{
  recursive_mutex::scoped_lock lock (m_mutex);
  TableContainer::iterator it = m_table.find (interest);
  if (it != m_table.end())
    m_table.erase(it);
  time_t currentTime = time(0);
  m_table.insert (make_pair(interest, currentTime));
}

void SyncInterestTable::expireInterests()
{
  recursive_mutex::scoped_lock lock (m_mutex);
  time_t currentTime = time(0);
  TableContainer::iterator it = m_table.begin (); 
  while (it != m_table.end())
    {
    time_t timestamp = it->second;
    if (currentTime - timestamp > m_checkPeriod) {
      it = m_table.erase(it);
    }
    else
      ++it;
  }
}

void SyncInterestTable::periodicCheck ()
{
  while (m_running)
    {
      try
        {
          // cout << "enterSleep: " << this_thread::get_id () << endl;
      
          this_thread::sleep (posix_time::seconds(4));
          expireInterests ();
        }
      catch (boost::thread_interrupted e)
        {
          // should I just assign m_running = false here?
          
          // cout << "interrupted: " << this_thread::get_id () << endl;
          // do nothing
        }
    }
}

}
