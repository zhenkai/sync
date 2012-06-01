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
 *         Chaoyi Bian <bcy@pku.edu.cn>
 *	   Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 */

#include <boost/test/unit_test.hpp>
#include <boost/test/output_test_stream.hpp> 
using boost::test_tools::output_test_stream;

#include <boost/make_shared.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include "sync-log.h"

#include "sync-app-socket.h"
extern "C" {
#include <unistd.h>
}

using namespace Sync;
using namespace std;
using namespace boost;

INIT_LOGGER ("Test.AppSocket");

class TestSocketApp {
public:
  map<string, string> data;
  void set(string str1, string str2) {
    // _LOG_FUNCTION (this << ", " << str1);
    data.insert(make_pair(str1, str2));
    // cout << str1 << ", " << str2 << endl;
  }

  void fetchAll(const vector<MissingDataInfo> &v, SyncAppSocket *socket) {
    int n = v.size();
    for (int i = 0; i < n; i++) {
      SeqNo s = v[i].low;
      for(++s; s <= v[i].high; ++s) {
        socket->fetchString(v[i].prefix, s, bind(&TestSocketApp::set, this, _1, _2));
      }
    }
  }

  void pass(const string &prefix) {
  }

  string toString(){
    map<string, string>::iterator it = data.begin(); 
    string str = "\n";
    for (; it != data.end(); ++it){
      str += "<";
      str += it->first;
      str += "|";
      str += it->second;
      str += ">";
      str += "\n";
    }

    return str;
  }

};

BOOST_AUTO_TEST_CASE (AppSocketTest)
{
  INIT_LOGGERS ();
  
  TestSocketApp a1, a2, a3;
	
  string syncPrefix("/let/us/sync");
  string p1("/irl.cs.ucla.edu"), p2("/yakshi.org"), p3("/google.com");

  _LOG_DEBUG ("s1");
  SyncAppSocket s1 (syncPrefix, bind(&TestSocketApp::fetchAll, &a1, _1, _2), bind(&TestSocketApp::pass, &a1, _1));
  this_thread::sleep (posix_time::milliseconds (50));
  _LOG_DEBUG ("s2");
  SyncAppSocket s2 (syncPrefix, bind(&TestSocketApp::fetchAll, &a2, _1, _2), bind(&TestSocketApp::pass, &a2, _1));
  this_thread::sleep (posix_time::milliseconds (50));
  SyncAppSocket s3 (syncPrefix, bind(&TestSocketApp::fetchAll, &a3, _1, _2), bind(&TestSocketApp::pass, &a3, _1));
  this_thread::sleep (posix_time::milliseconds (50));

  // single source
  string data0 = "Very funny Scotty, now beam down my clothes";
  _LOG_DEBUG ("s1 publish");
  s1.publishString (p1, 0, data0, 10); 
  this_thread::sleep (posix_time::milliseconds (120));

  // from code logic, we won't be fetching our own data
  a1.set(p1 + "/0/0", data0);
  BOOST_CHECK_EQUAL(a1.toString(), a2.toString());
  BOOST_CHECK_EQUAL(a2.toString(), a3.toString());

  // single source, multiple data at once
  string data1 = "Yes, give me that ketchup";
  string data2 = "Don't look conspicuous, it draws fire";

  _LOG_DEBUG ("s1 publish");
  s1.publishString (p1, 0, data1, 10);
  _LOG_DEBUG ("s1 publish");
  s1.publishString (p1, 0, data2, 10);
  this_thread::sleep (posix_time::milliseconds (1000));
  
  // from code logic, we won't be fetching our own data
  a1.set(p1 + "/0/1", data1);
  a1.set(p1 + "/0/2", data2);
  BOOST_CHECK_EQUAL(a1.toString(), a2.toString());
  BOOST_CHECK_EQUAL(a2.toString(), a3.toString());

  // another single source
  string data3 = "You surf the Internet, I surf the real world";
  string data4 = "I got a fortune cookie once that said 'You like Chinese food'";
  string data5 = "Real men wear pink. Why? Because their wives make them";
  _LOG_DEBUG ("s3 publish");
  s3.publishString(p3, 0, data3, 10); 
  this_thread::sleep (posix_time::milliseconds (200));
  
  // another single source, multiple data at once
  s2.publishString(p2, 0, data4, 10); 
  s2.publishString(p2, 0, data5, 10);
  this_thread::sleep (posix_time::milliseconds (1000));

  // from code logic, we won't be fetching our own data
  a3.set(p3 + "/0/0", data3);
  a2.set(p2 + "/0/0", data4);
  a2.set(p2 + "/0/1", data5);
  BOOST_CHECK_EQUAL(a1.toString(), a2.toString());
  BOOST_CHECK_EQUAL(a2.toString(), a3.toString());

  // not sure weither this is simultanous data generation from multiple sources
  _LOG_DEBUG ("Simultaneous publishing");
  string data6 = "Shakespeare says: 'Prose before hos.'";
  string data7 = "Pick good people, talent never wears out";
  s1.publishString(p1, 0, data6, 10); 
  // this_thread::sleep (posix_time::milliseconds (1000));
  s2.publishString(p2, 0, data7, 10); 
  this_thread::sleep (posix_time::milliseconds (1500));

  // from code logic, we won't be fetching our own data
  a1.set(p1 + "/0/3", data6);
  a2.set(p2 + "/0/2", data7);
  // a1.set(p1 + "/0/1", data6);
  // a2.set(p2 + "/0/0", data7);
  BOOST_CHECK_EQUAL(a1.toString(), a2.toString());
  BOOST_CHECK_EQUAL(a2.toString(), a3.toString());

  _LOG_DEBUG ("Finish");
}
