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
using boost::test_tools::output_test_stream;

#include <boost/make_shared.hpp>

#include "../model/sync-app-socket.h"
extern "C" {
#include <unistd.h>
}

using namespace Sync;
using namespace std;
using namespace boost;

class TestStructApp {
public:
  map<string, string> data;
  void set(string str1, string str2) {
    data.insert(make_pair(str1, str2));
  }

  string toString(){
    map<string, string>::iterator it = data.begin(); 
    string str = "";
    for (; it != data.end(); ++it){
      str += "<";
      str += it->first;
      str += "|";
      str += it->second;
      str += ">";
    }
    return str;
  }

};

BOOST_AUTO_TEST_CASE (AppSocketTest)
{

	TestStructApp a1, a2, a3;
	boost::function<void(string, string)> f1 = bind(&TestStructApp::set, &a1, _1,
	_2);
	boost::function<void(string, string)> f2 = bind(&TestStructApp::set, &a2, _1,
	_2);
	boost::function<void(string, string)> f3 = bind(&TestStructApp::set, &a3, _1,
	_2);
	
	string p1("/irl.cs.ucla.edu"), p2("/yakshi.org"), p3("/google.com");

	SyncAppSocket s1(p1, f1), s2(p2, f2), s3(p3, f3);
	BOOST_CHECK_EQUAL(a1.toString(), a2.toString());
	BOOST_CHECK_EQUAL(a2.toString(), a3.toString());

	// single source
	s1.publish(p1, 0, "Very funny Scotty, now beam down my clothes", 10); 
	usleep(10000);
	BOOST_CHECK_EQUAL(a1.toString(), a2.toString());
	BOOST_CHECK_EQUAL(a2.toString(), a3.toString());

	// single source, multiple data at once
	s1.publish(p1, 0, "Yes, give me that ketchup", 10); 
	s1.publish(p1, 0, "Don't look conspicuous, it draws fire", 10); 
	usleep(10000);
	BOOST_CHECK_EQUAL(a1.toString(), a2.toString());
	BOOST_CHECK_EQUAL(a2.toString(), a3.toString());

	// another single source
	s3.publish(p3, 0, "You surf the Internet, I surf the real world", 10); 
	usleep(10000);
	// another single source, multiple data at once
	s2.publish(p2, 0, "I got a fortune cookie once that said 'You like Chinese food'", 10); 
	s2.publish(p2, 0, "Real men wear pink. Why? Because their wives make them", 10);
	usleep(10000);
	BOOST_CHECK_EQUAL(a1.toString(), a2.toString());
	BOOST_CHECK_EQUAL(a2.toString(), a3.toString());

	// not sure weither this is simultanous data generation from multiple sources
	s1.publish(p1, 0, "Shakespeare says: 'Prose before hos.'", 10); 
	s2.publish(p2, 0, "Pick good people, talent never wears out", 10); 
	usleep(10000);
	BOOST_CHECK_EQUAL(a1.toString(), a2.toString());
	BOOST_CHECK_EQUAL(a2.toString(), a3.toString());
}


