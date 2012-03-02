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

#ifndef SYNC_NAME_INFO_H
#define SYNC_NAME_INFO_H

#include <boost/shared_ptr.hpp>
#include <map>
#include <string>

namespace ns3 {
namespace Sync {

/**
 * @ingroup sync
 * @brief Templated class for the leaf name
 */
class NameInfo
{
private:
  typedef boost::shared_ptr<const NameInfo> const_ptr;
  
public:
  virtual ~NameInfo () { };

  /**
   * @brief Get ID of the record (should be locally-unique, but not really necessary---this is be used for hashing purposes)
   */
  size_t
  getHashId () const { return m_id; }
  
  virtual bool
  operator == (const NameInfo &info) const = 0;

  /**
   * @brief Calculates digest of the name
   */
  const Digest &
  getDigest () const { return m_digest; }

  /**
   * @brief Convert prefix to string
   * @returns string representation of prefix
   */
  virtual std::string
  toString () const = 0;

protected:
  // actual stuff
  size_t m_id; ///< @brief Identifies NameInfo throughout the library (for hash container, doesn't need to be strictly unique)
  Digest m_digest;

  // static stuff
  typedef std::map<std::string, const_ptr> NameMap;
  static size_t  m_ids;
  static NameMap m_names;
};

typedef boost::shared_ptr<NameInfo> NameInfoPtr;
typedef boost::shared_ptr<const NameInfo> NameInfoConstPtr;


} // Sync
} // ns3

#endif // SYNC_NAME_INFO_H
