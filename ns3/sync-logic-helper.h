/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 University of California, Los Angeles
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
 * Author: Alexander Afanasyev <alexander.afanasyev@ucla.edu>
 */

#ifndef SYNC_LOGIC_HELPER_H
#define SYNC_LOGIC_HELPER_H

#include "ns3/object-factory.h"
#include "ns3/attribute.h"
#include "ns3/node-container.h"
#include "ns3/application-container.h"
#include "ns3/ptr.h"

#include <boost/function.hpp>

namespace Sync 
{

class SeqNo;

/**
 * \brief A helper to make it easier to instantiate an ns3::CcnxConsumer Application
 * on a set of nodes.
 */
class SyncLogicHelper
{        
public:
  typedef boost::function< void ( const std::string &/*prefix*/, const SeqNo &/*newSeq*/, const SeqNo &/*oldSeq*/ ) > LogicUpdateCallback;
  typedef boost::function< void ( const std::string &/*prefix*/ ) > LogicRemoveCallback;

  /**
   * \brief Create an CcnxAppHelper to make it easier to work with CCNx apps
   *
   * \param app Class of the application
   */
  SyncLogicHelper ();

  /**
   * @brief Set the sync prefix
   */
  void
  SetPrefix (const std::string &prefix);

  /**
   * @brief Set onUpdate and onRemove callbacks
   */
  void
  SetCallbacks (LogicUpdateCallback onUpdate, LogicRemoveCallback onRemove);
  
  /**
   * Install an ns3::CcnxConsumer on each node of the input container
   * configured with all the attributes set with SetAttribute.
   *
   * \param c NodeContainer of the set of nodes on which an CcnxConsumer 
   * will be installed.
   * \returns Container of Ptr to the applications installed.
   */
  ns3::ApplicationContainer
  Install (ns3::NodeContainer c);
        
  /**
   * Install an ns3::CcnxConsumer on the node configured with all the 
   * attributes set with SetAttribute.
   *
   * \param node The node on which an CcnxConsumer will be installed.
   * \returns Container of Ptr to the applications installed.
   */
  ns3::ApplicationContainer
  Install (ns3::Ptr<ns3::Node> node);
        
  /**
   * Install an ns3::CcnxConsumer on the node configured with all the 
   * attributes set with SetAttribute.
   *
   * \param nodeName The node on which an CcnxConsumer will be installed.
   * \returns Container of Ptr to the applications installed.
   */
  ns3::ApplicationContainer
  Install (std::string nodeName);
        
private:
  /**
   * \internal
   * Install an ns3::CcnxConsumer on the node configured with all the 
   * attributes set with SetAttribute.
   *
   * \param node The node on which an CcnxConsumer will be installed.
   * \returns Ptr to the application installed.
   */
  ns3::Ptr<ns3::Application> InstallPriv (ns3::Ptr<ns3::Node> node);
  std::string m_prefix; // sync prefix
  LogicUpdateCallback m_onUpdate;
  LogicRemoveCallback m_onRemove;
};

} // namespace Sync

#endif // SYNC_LOGIC_HELPER_H

