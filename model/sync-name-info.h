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

namespace ns3 {
namespace Sync {

/**
 * @brief Abstraction of the leaf name
 */
class NameInfo
{
public:
  virtual ~NameInfo () = 0;
  /**
   * @brief Calculates digest of the name
   */
  // Digest
  // getDigest () const;

  virtual std::string
  toString () const = 0;
};

typedef boost::shared_ptr<NameInfo> NameInfoPtr;
typedef boost::shared_ptr<const NameInfo> NameInfoConstPtr;

} // Sync
} // ns3

#endif // SYNC_NAME_INFO_H
