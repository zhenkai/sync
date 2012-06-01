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

#ifndef STANDALONE

#ifndef SYNC_CCNX_NAME_INFO_H
#define SYNC_CCNX_NAME_INFO_H

#include "sync-name-info.h"
#include "ns3/ptr.h"
#include "ns3/ccnx-name-components.h"

namespace Sync {

class Ns3NameInfo : public NameInfo
{
public:
  /**
   * @brief Lookup existing or create new NameInfo object
   * @param name routable prefix
   */
  static NameInfoConstPtr
  FindOrCreate (ns3::Ptr<const ns3::CcnxNameComponents> name);

  virtual ~Ns3NameInfo () { };
  
  // from NameInfo
  virtual bool
  operator == (const NameInfo &info) const;

  virtual bool
  operator < (const NameInfo &info) const;

  virtual std::string
  toString () const;

private:
  // implementing a singleton pattern. 
  /**
   * @brief Disabled default constructor. NameInfo object should be created through FindOrCreate static call.
   */

  /**
   * @brief Disabled default
   */
  Ns3NameInfo () {}
  Ns3NameInfo& operator = (const Ns3NameInfo &info) { return *this; }
  Ns3NameInfo (ns3::Ptr<const ns3::CcnxNameComponents> name);
  
  ns3::Ptr<const ns3::CcnxNameComponents> m_name;
};

Digest &
operator << (Digest &, const ns3::CcnxNameComponents &name);

} // Sync

#endif // SYNC_CCNX_NAME_INFO_H

#endif // STANDALONE
