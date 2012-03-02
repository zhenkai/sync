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

#ifndef SYNC_CCNX_NAME_INFO_H
#define SYNC_CCNX_NAME_INFO_H

#include "sync-name-info.h"
#include "ns3/ptr.h"

namespace ns3 {

class CcnxNameComponents;

namespace Sync {

class CcnxNameInfo : public NameInfo
{
public:
  /**
   * @brief Lookup existing or create new NameInfo object
   * @param name routable prefix
   */
  static NameInfoConstPtr
  FindOrCreate (Ptr<const CcnxNameComponents> name);

  virtual ~CcnxNameInfo () { };
  
  // from NameInfo
  virtual bool
  operator == (const NameInfo &info) const;

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
  CcnxNameInfo () {}
  CcnxNameInfo& operator = (const CcnxNameInfo &info) { return *this; }
  CcnxNameInfo (Ptr<const CcnxNameComponents> name);
  
  Ptr<const CcnxNameComponents> m_name;
};

} // Sync
} // ns3

#endif // SYNC_CCNX_NAME_INFO_H
