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

#include "sync-logic-helper.h"
#include "sync-logic.h"

#include "ns3/log.h"
#include "ns3/string.h"
#include "ns3/names.h"

NS_LOG_COMPONENT_DEFINE ("SyncLogicHelper");

using namespace ns3;

namespace Sync 
{

SyncLogicHelper::SyncLogicHelper ()
{
  // m_factory.SetTypeId ("Sync::SyncLogic");
}

void
SyncLogicHelper::SetPrefix (const std::string &prefix)
{
  m_prefix = prefix;
}


void
SyncLogicHelper::SetCallbacks (LogicUpdateCallback onUpdate, LogicRemoveCallback onRemove)
{
  m_onUpdate = onUpdate;
  m_onRemove = onRemove;
}

// void 
// CcnxAppHelper::SetAttribute (std::string name, const AttributeValue &value)
// {
//   m_factory.Set (name, value);
// }
    
ApplicationContainer
SyncLogicHelper::Install (Ptr<Node> node)
{
  return ApplicationContainer (InstallPriv (node));
}
    
ApplicationContainer
SyncLogicHelper::Install (std::string nodeName)
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}
    
ApplicationContainer
SyncLogicHelper::Install (NodeContainer c)
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }
    
  return apps;
}
    
Ptr<Application>
SyncLogicHelper::InstallPriv (Ptr<Node> node)
{
  Ptr<SyncLogic> app = CreateObject<SyncLogic> ("/sync", m_onUpdate, m_onRemove);
  node->AddApplication (app);
        
  return app;
}

}
