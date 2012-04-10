/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/NDNabstraction-module.h>
#include <ns3/point-to-point-module.h>

#include "sync-logic.h"
#include "sync-logic-helper.h"

using namespace ns3;
using namespace Sync;

NS_LOG_COMPONENT_DEFINE ("SyncExample");

void OnUpdate (const std::string &prefix, const SeqNo &newSeq, const SeqNo &/*oldSeq*/)
{
  NS_LOG_LOGIC (Simulator::Now ().ToDouble (Time::S) <<"s\tNode: " << Simulator::GetContext () << ", prefix: " << prefix << ", seqNo: " << newSeq);
}

void OnRemove (const std::string &prefix)
{
  NS_LOG_LOGIC (Simulator::Now ().ToDouble (Time::S) <<"s\tNode: " << Simulator::GetContext () << ", prefix: "<< prefix);
}

int 
main (int argc, char *argv[])
{
  Config::SetDefault ("ns3::PointToPointNetDevice::DataRate", StringValue ("1Mbps"));
  Config::SetDefault ("ns3::PointToPointChannel::Delay", StringValue ("10ms"));
  Config::SetDefault ("ns3::DropTailQueue::MaxPackets", StringValue ("20"));
  
  Config::SetDefault ("ns3::CcnxFloodingStrategy::SmartFlooding", StringValue ("false"));

  LogComponentEnable ("SyncExample", LOG_ALL);
  
  Time finishTime = Seconds (3.0); 

  CommandLine cmd;
  cmd.AddValue ("finish", "Finish time", finishTime);
  cmd.Parse (argc, argv);

  // Creating nodes
  NodeContainer nodes;
  nodes.Create (3);

  // Connecting nodes using two links
  PointToPointHelper p2p;
  p2p.Install (nodes.Get (0), nodes.Get (1));
  p2p.Install (nodes.Get (1), nodes.Get (2));

  Names::Add ("0", nodes.Get (0));
  Names::Add ("1", nodes.Get (1));
  Names::Add ("2", nodes.Get (2));
  
  // Install CCNx stack on all nodes
  NS_LOG_INFO ("Installing CCNx stack");
  CcnxStackHelper ccnxHelper;
  ccnxHelper.SetForwardingStrategy ("ns3::CcnxFloodingStrategy");
  ccnxHelper.SetDefaultRoutes (false);
  ccnxHelper.InstallAll ();

  ccnxHelper.AddRoute ("0", "/sync", 0, 0);
  ccnxHelper.AddRoute ("1", "/sync", 0, 0);
  ccnxHelper.AddRoute ("1", "/sync", 1, 0);
  ccnxHelper.AddRoute ("2", "/sync", 0, 0);

  SyncLogicHelper logicHelper;
  logicHelper.SetPrefix ("/sync");
  logicHelper.SetCallbacks (OnUpdate, OnRemove);
  ApplicationContainer apps = logicHelper.Install (NodeContainer (nodes.Get (0), nodes.Get (1)));
  
  // one data
  Simulator::ScheduleWithContext (0, Seconds (0.5), &SyncLogic::addLocalNames, DynamicCast<SyncLogic> (apps.Get (0)), "/0", 1, 1);

  // two producers at the same time
  Simulator::ScheduleWithContext (0, Seconds (1.001), &SyncLogic::addLocalNames, DynamicCast<SyncLogic> (apps.Get (0)), "/0", 1, 2);
  Simulator::ScheduleWithContext (1, Seconds (1.001), &SyncLogic::addLocalNames, DynamicCast<SyncLogic> (apps.Get (1)), "/1", 1, 2);

  logicHelper.Install (nodes.Get (2)).
    Start (Seconds (2.001));
  
  Simulator::Stop (finishTime);
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}


