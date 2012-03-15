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

#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp> 
#include <map>
using boost::test_tools::output_test_stream;

#include <boost/make_shared.hpp>

#include "../model/sync-ccnx-wrapper.h"
#include "../model/sync-logic.h"
#include "../model/sync-seq-no.h"

using namespace std;
using namespace boost;
using namespace Sync;

struct Handler
{
  string instance;
  
  Handler (const string &_instance)
  : instance (_instance)
  {
  }
  
  void onUpdate (const string &p/*prefix*/, const SeqNo &seq/*newSeq*/, const SeqNo &oldSeq/*oldSeq*/)
  {
    m_map[p] = seq.getSeq ();
    
    // cout << instance << "\t";
    // if (!oldSeq.isValid ())
    //   cout << "Inserted: " << p << " (" << seq << ")" << endl;
    // else
    //   cout << "Updated: " << p << "  ( " << oldSeq << ".." << seq << ")" << endl;
  }

  void onRemove (const string &p/*prefix*/)
  {
    // cout << instance << "\tRemoved: " << p << endl;
    m_map.erase (p);
  }

  map<string, uint32_t> m_map;
};

BOOST_AUTO_TEST_CASE (SyncLogicTest)
{
  Handler h1 ("1");

  SyncLogic l1 ("/bcast", bind (&Handler::onUpdate, &h1, _1, _2, _3), bind (&Handler::onRemove, &h1, _1));
  l1.addLocalNames ("/one", 1, 2);

  BOOST_CHECK_EQUAL (h1.m_map.size (), 0);
  sleep (1);
  BOOST_CHECK_EQUAL (h1.m_map.size (), 0);

  Handler h2 ("2");
  SyncLogic l2 ("/bcast", bind (&Handler::onUpdate, &h2, _1, _2, _3), bind (&Handler::onRemove, &h2, _1));
  
  sleep (1);
  BOOST_CHECK_EQUAL (h1.m_map.size (), 0);
  BOOST_CHECK_EQUAL (h2.m_map.size (), 1);
  
  // l1.remove ("/one");

  // sleep (10);
// l1.  
}
