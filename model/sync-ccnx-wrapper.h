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

extern "C" {
#include <ccn/ccn.h>
#include <ccn/charbuf.h>
#include <ccn/keystore.h>
#include <ccn/uri.h>
#include <ccn/bloom.h>
}

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <string>
/**
 * \defgroup sync SYNC protocol
 *
 * Implementation of SYNC protocol
 */
namespace Sync {
/**
 * \ingroup sync
 * @brief Data wrapper with size
 */
 class DataBuffer {
private:
	unsigned char *m_buffer;
	size_t m_len;
public:
	DataBuffer(const unsigned char *buffer, size_t len);
	DataBuffer(const DataBuffer &dataBuffer);
	DataBuffer &operator=(const DataBuffer &dataBuffer);
	~DataBuffer();	
	size_t length() {return len;}
	const unsigned char *buffer() { return const_cast<const unsigned char *> (buffer); }
};

/**
 * \ingroup sync
 * @brief A wrapper for ccnx library; clients of this code do not need to deal
 * with ccnx library
 */
class CcnxWrapper { 
private:
	ccn* m_handle;
	ccn_keystore *m_keyStore;
	ccn_charbuf *m_keyLoactor;
	boost::mutex m_mutex;
	boost::shared_ptr<boos::thread> m_thread;

private:
	void createKeyLocator();
	void initKeyStore();
	const ccn_pkey *getPrivateKey();
	const unsigned char *getPublicKeyDigest();
	ssize_t getPublicKeyDigestLength();
	void ccnLoop();

public:
  

   CcnxWrapper();
   ~CcnxWrapper();
  /**
   * @brief send Interest 
   *
   * @param strInterest the Interest name
   * @param callback the callback function to deal with the returned data
   */
   int sendInterest(std::string strInterest, boost::function<void (DataBuffer &)> processData);
   int sendInterestFilter(std::string prefix, boost::function<void (std::string)>
   processInterest);
   int publishData(std::string name, DataBuffer &dataBuffer);

};

} // Sync

#endif // SYNC_STATE_H
