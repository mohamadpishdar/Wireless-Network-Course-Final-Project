/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 */

#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/applications-module.h"
#include "ns3/energy-module.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Wifi0");

void
TagMarker (uint8_t tid, Ptr<const Packet> packet)
{
 QosTag qosTag;
 qosTag.SetTid(tid);
 packet->AddPacketTag (qosTag);
}

double PhyRxSum=0;


void PhyRx(Ptr<const Packet> p)
{
    if(Simulator::Now().GetSeconds()>=0.5)
    {
        PhyRxSum=PhyRxSum+p->GetSize();
    }
}


int main (int argc, char *argv[])
{

    uint32_t nStaNodes=5;
    bool mobile=true;
    uint32_t psize=100;


    //create the nodes
    NodeContainer apNodes;
    NodeContainer staNodes;
    apNodes.Create(1);
    staNodes.Create(nStaNodes);

    //create the wifi connection
    WifiHelper wifi;
    wifi.SetStandard(WIFI_PHY_STANDARD_80211g);

    QosWifiMacHelper wifiMac =QosWifiMacHelper::Default ();

wifi.SetRemoteStationManager ("ns3::IdealWifiManager");
    YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
//Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("0"));

    //set attributes of the channel
    YansWifiChannelHelper channel=YansWifiChannelHelper::Default ();
    phy.SetChannel (channel.Create ());

  wifiMac.SetMsduAggregatorForAc (AC_BK, "ns3::MsduStandardAggregator", "MaxAmsduSize", UintegerValue (8*1024));
   wifiMac.SetBlockAckThresholdForAc (AC_BK,2);
    //install the wifi to the nodes
    Ssid ssid = Ssid ("wifi-default");
    wifiMac.SetType ("ns3::ApWifiMac","Ssid", SsidValue (ssid));
    NetDeviceContainer apDevice = wifi.Install (phy, wifiMac, apNodes);
    wifiMac.SetType ("ns3::StaWifiMac", "Ssid", SsidValue (ssid),"ActiveProbing", BooleanValue (false));
    NetDeviceContainer staDevice = wifi.Install (phy, wifiMac, staNodes);

    //set mobility model and position of nodes, a node is at (0,0,0) and doesn't move
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
    positionAlloc->Add (Vector (0.0, 0.0, 0.0));
    MobilityHelper mobility;
    mobility.SetPositionAllocator(positionAlloc);
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install(apNodes);

    //the sta nodes start from a fixed position and they move randomly if chosen so
    Ptr<ListPositionAllocator> positionAlloc1 = CreateObject<ListPositionAllocator> ();
    for(uint32_t cnt=1;cnt<nStaNodes+1;cnt++)
    {
        positionAlloc1->Add (Vector (25, 0, 0));
        positionAlloc1->Add (Vector (-25, 0, 0));
        positionAlloc1->Add (Vector (0, -25, 0));
        positionAlloc1->Add (Vector (0, 25, 0));
        positionAlloc1->Add (Vector (18, -18, 0));
        positionAlloc1->Add (Vector (-18, -18, 0));
        positionAlloc1->Add (Vector (-18, 18, 0));
        positionAlloc1->Add (Vector (18, 18, 0));
        positionAlloc1->Add (Vector (-13, -13, 0));
        positionAlloc1->Add (Vector (13, 13, 0));
        positionAlloc1->Add (Vector (-13, 13, 0));
        positionAlloc1->Add (Vector (13, -13, 0));
        positionAlloc1->Add (Vector (0, 20, 0));
        positionAlloc1->Add (Vector (20, 0, 0));
        positionAlloc1->Add (Vector (-20, 0, 0));
        positionAlloc1->Add (Vector (0, -20, 0));
        positionAlloc1->Add (Vector (-30, 30, 0));
        positionAlloc1->Add (Vector (30, -30, 0));
        positionAlloc1->Add (Vector (-30, -30, 0));
        positionAlloc1->Add (Vector (30, 30, 0));
    }

    MobilityHelper mobility1;
    mobility1.SetPositionAllocator (positionAlloc1);
    if(mobile==true)
    {
        mobility1.SetMobilityModel ("ns3::RandomWalk2dMobilityModel","Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));
    }
    else
    {
        mobility1.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    }
    mobility1.Install(staNodes);

    //install internet stack and addresses of nodes
    InternetStackHelper istack;
    istack.Install (apNodes);
    istack.Install (staNodes);

    Ipv4AddressHelper address;
    address.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer j =address.Assign (apDevice);
    Ipv4InterfaceContainer i =address.Assign (staDevice);

    //install on-off application to odd nodes
    uint16_t port= 9;
    ApplicationContainer sourceApps[nStaNodes];
    OnOffHelper OnOff=OnOffHelper("ns3::UdpSocketFactory", InetSocketAddress (j.GetAddress(0,0), port));
    OnOff.SetAttribute ("DataRate",DataRateValue (DataRate ("65Mbps")));
    OnOff.SetAttribute("PacketSize",UintegerValue(psize));
    OnOff.SetAttribute("OnTime",StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
    OnOff.SetAttribute("OffTime",StringValue ("ns3::ConstantRandomVariable[Constant=0]"));

    for(uint32_t n=0;n<nStaNodes;n=n+1)
    {

        sourceApps[n] = OnOff.Install (staNodes.Get (n));


        Ptr<OnOffApplication> onoffapp2;
         onoffapp2 = DynamicCast<OnOffApplication>(sourceApps[n].Get(0));
        onoffapp2->TraceConnectWithoutContext("Tx",  MakeBoundCallback (&TagMarker, AC_BK));
         sourceApps[n].Start (Seconds (0));
         sourceApps[n].Stop (Seconds (30.0));

    }

    //install packet sink application to even nodes
    PacketSinkHelper sink ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny(), port));
    ApplicationContainer sinkApps;

        sinkApps=sink.Install (apNodes.Get (0));
        sinkApps.Start (Seconds (0));
        sinkApps.Stop (Seconds (30.0));


    //make the routing table
    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

    Config::ConnectWithoutContext("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyRxEnd", MakeCallback(&PhyRx));

    Simulator::Stop (Seconds (5.0));
    Simulator::Run ();

     NS_LOG_UNCOND ("Total Average Throughput phy Per Mobile Node:" << (PhyRxSum*8/nStaNodes)/30000000) ;


    Simulator::Destroy ();
    return 0;
}
