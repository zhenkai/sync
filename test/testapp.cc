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

#include "../model/sync-digest.h"
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <sstream>

using namespace Sync;
using namespace std;
using namespace boost;

int
main (int argc, char **argv)
{
  Digest test;
  test << "1\n";

  // try
  //   {
  //     cout << "Trying to print without explicit getHash() call: ";
  //     cout << test;
  //     cout << "Failed (should be asserted)\n";
  //   }
  // catch (...)
  //   {
  //     cout << "OK (exception)\n";
  //   }
  
  // without explicit finalizing, Digest will not be complete and printing out will cause assert
  test.getHash ();

  try
    {
      cout << "Hash for '1' is " << test << std::endl;
    }
  catch (...)
    {
      cout << "Hash calculation failed\n";
    }

  try
    {
      cout << "Trying to add data to hash after finalizing: ";
      test << "2"; // should cause an assert
      cout << "Failed (should be asserted)\n";
    }
  catch (...)
    {
      cout << "OK (exception)\n";
    }

  Digest test2;
// #ifdef DIGEST_BASE64
//   string testString = "sCYyTGkEsqnLS4jW1hyB0Q==";
// #else
  string testString = lexical_cast<string> (test); //"b026324c6904b2a9cb4b88d6d61c81d1";
// #endif
  cout << "Hash from string: " << testString << "\n";
  istringstream is (testString);
  is >> test2;

  cout << "Result from hash: " << test2 << "\n";

  cout << "Compare two hashes: " << (test == test2) << "\n";

  Digest test3;
  if (testString[0] != '1')
    testString[0] = '1';
  else
    testString[0] = '2';
  
  istringstream is2(testString);
  is2 >> test3;

  cout << "Hash from string: " << test3 << "\n";
  cout << "Compare two hashes: " << (test == test3) << "\n";
  
  return 0; 
}
