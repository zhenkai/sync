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

#include "../model/sync-interest-table.h"
#include "../model/sync-logic.h"

using namespace Sync;
using namespace std;
using namespace boost;

BOOST_AUTO_TEST_CASE (InterestTableTest)
{
  SyncInterestTable *table = 0;
  BOOST_CHECK_NO_THROW (table = new SyncInterestTable ());

  BOOST_CHECK_NO_THROW (delete table);
}

void funcUpdate (const std::string &, const SeqNo &newSeq, const SeqNo &oldSeq)
{
  cout << "funcUpdate\n";
}

void funcRemove (const std::string &)
{
  cout << "funcRemove\n";
}

BOOST_AUTO_TEST_CASE (SyncLogicTest)
{  
  SyncLogic *logic = 0;
  BOOST_CHECK_NO_THROW (logic = new SyncLogic ("/prefix", funcUpdate, funcRemove, make_shared<CcnxWrapper> ()));
  BOOST_CHECK_EQUAL (logic->getListChecksSize (), 0);

  // 0s
  BOOST_CHECK_NO_THROW (logic->respondSyncInterest ("/prefix/e5fa44f2b31c1fb553b6021e7360d07d5d91ff5e")); 
  sleep (1);

  // 1s
  BOOST_CHECK_NO_THROW (logic->respondSyncInterest ("/prefix/e5fa44f2b31c1fb553b6021e7360d07d5d91ff5e")); 
  sleep (1);

  // 2s
  // cout << "Wait queue size: " << logic->getListChecksSize () << endl;
  BOOST_CHECK_EQUAL (logic->getListChecksSize (), 1);

  // 2s
  BOOST_CHECK_NO_THROW (logic->respondSyncInterest ("/prefix/e5fa44f2b31c1fb553b6021e7360d07d5d91ff5e"));  
  // cout << "Wait queue size: " << logic->getListChecksSize () << endl;
  BOOST_CHECK_EQUAL (logic->getListChecksSize (), 2);

  this_thread::sleep (posix_time::milliseconds (2500)); // make two interests expire

  // 4.5s
  // cout << "(after 3.3s) Wait queue size: " << logic->getListChecksSize () << endl;
  BOOST_CHECK_EQUAL (logic->getListChecksSize (), 1);

  BOOST_CHECK_NO_THROW (logic->respondSyncInterest ("/prefix/e5fa44f2b31c1fb553b6021e7360d07d5d91ff5e"));
  sleep (5);
  BOOST_CHECK_EQUAL (logic->getListChecksSize (), 0);
  
  BOOST_CHECK_NO_THROW (delete logic);
}

