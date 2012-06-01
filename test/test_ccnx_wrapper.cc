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

#include "sync-ccnx-wrapper.h"

using namespace Sync;
using namespace std;
using namespace boost;

string echoStr = "";

void echo(string str) {
  echoStr = str;
}

struct TestStruct {
  string s_str1, s_str2;
  void set(string str1, string str2) {
    s_str1 = str1;
    s_str2 = str2;
  }
  char *m_buf;
  size_t m_len;

  void rawSet(string str1, const char *buf, size_t len) {
    std::cout << "In rawSet" << std::endl;
    m_buf = (char  *)calloc(1, len);
    memcpy(m_buf, buf, len);
    s_str1 = str1;
  }
};

BOOST_AUTO_TEST_CASE (CcnxWrapperTest)
{
  CcnxWrapper ha;
  CcnxWrapper hb;
	
  TestStruct foo;
	
  boost::function<void (string)> globalFunc = echo;
  boost::function<void (string, string)> memberFunc =
    bind(&TestStruct::set, &foo, _1, _2);

  boost::function<void (string, const char *, size_t)> rawFunc =
    bind(&TestStruct::rawSet, &foo, _1, _2, _3);

  string prefix = "/ucla.edu";
  ha.setInterestFilter(prefix, globalFunc);
  this_thread::sleep (posix_time::milliseconds (10));

  string interest = "/ucla.edu/0";
  hb.sendInterestForString(interest, memberFunc);

  // give time for ccnd to react
  sleep(1);
  this_thread::sleep (posix_time::milliseconds (5));
  BOOST_CHECK_EQUAL(echoStr, interest);

  string name = "/ucla.edu/0";
  string data = "random bits: !#$!@#$!";
  ha.publishStringData(name, data, 5);

  hb.sendInterestForString(interest, memberFunc);

  // give time for ccnd to react
  this_thread::sleep (posix_time::milliseconds (5));
  BOOST_CHECK_EQUAL(foo.s_str1, name);
  BOOST_CHECK_EQUAL(foo.s_str2, data);

  string rawDataName = "/ucla.edu/1";
  int num[5] = {0, 1, 2, 3, 4};
  ha.publishRawData(rawDataName, (const char *)num, sizeof(num), 30);
  hb.sendInterest(rawDataName, rawFunc);
  BOOST_CHECK_EQUAL(foo.s_str1, rawDataName);
  
  BOOST_CHECK(memcmp((char *)num, foo.m_buf, foo.m_len) == 0);
}


