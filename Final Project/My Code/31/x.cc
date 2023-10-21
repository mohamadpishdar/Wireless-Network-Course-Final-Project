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

double PhyRxSum=0;
double MacRxSum=0;
double  totalRxBytesCounter=0;
ApplicationContainer sinkApp;


void ReceivePacket (Ptr<const Packet> packet, const Address &)
{

    totalRxBytesCounter += packet->GetSize ();
        totalRxBytesCounter+= 1;


// NS_LOG_UNCOND ("count basteha daryaft shode " << count);
 //NS_LOG_UNCOND("th " << Simulator::Now ().GetSeconds()<< "\t"<<((totalRxBytesCounter*8.0)/1000000)/Simulator::Now ().GetSeconds());
 //NS_LOG_UNCOND("average packet transmition time " << Simulator::Now ().GetSeconds()<< "\t"<<((Simulator::Now ().GetSeconds())*1000)/count);


}


void PhyRx(Ptr<const Packet> p)
{
    if(Simulator::Now().GetSeconds()>=0.5)
    {
        PhyRxSum=PhyRxSum+p->GetSize();
    }
}

void MacRx(Ptr<const Packet> p)
{
    if(Simulator::Now().GetSeconds()>=0.5)
    {
        MacRxSum=MacRxSum+p->GetSize();
    }
}

void
TagMarker (uint8_t tid, Ptr<const Packet> packet)
{
 QosTag qosTag;
 qosTag.SetTid(tid);
 packet->AddPacketTag (qosTag);
}

int main (int argc, char *argv[])
{

    uint32_t nStaNodes=1;

    uint32_t psize=240;



    //create the nodes
    NodeContainer apNodes;
    NodeContainer staNodes;
    apNodes.Create(1);
    staNodes.Create(nStaNodes);

    //create the wifi connection
    WifiHelper wifi;
    wifi.SetStandard( WIFI_PHY_STANDARD_80211g);

   QosWifiMacHelper wifiMac = QosWifiMacHelper::Default ();

  // wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
    //                              "DataMode",StringValue ("DsssRate11Mbps"),
      //                          "ControlMode",StringValue ("DsssRate11Mbps"));
   wifi.SetRemoteStationManager ("ns3::AarfWifiManager");
    YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
    phy.SetErrorRateModel("ns3::YansErrorRateModel");

    //set attributes of the channel
    YansWifiChannelHelper channel= YansWifiChannelHelper::Default ();
  //  channel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  //  channel.AddPropagationLoss ("ns3::LogDistancePropagationLossModel","Exponent", DoubleValue (3.0));
    phy.SetChannel (channel.Create ());



    //install the wifi to the nodes
    Ssid ssid = Ssid ("wifi-default");
    wifiMac.SetType ("ns3::ApWifiMac","Ssid", SsidValue (ssid));
    NetDeviceContainer apDevice = wifi.Install (phy, wifiMac, apNodes);


    wifiMac.SetMsduAggregatorForAc (AC_BK, "ns3::MsduStandardAggregator", "MaxAmsduSize", UintegerValue (7935));
  // wifiMac.SetMpduAggregatorForAc (AC_BK,"ns3::MpduStandardAggregator", "MaxAmpduSize", UintegerValue (10240));
   // wifiMac.SetMpduAggregatorForAc (AC_VI,"ns3::MpduStandardAggregator", "MaxAmpduSize", UintegerValue (10240));
 //  wifiMac.SetMpduAggregatorForAc (AC_BE,"ns3::MpduStandardAggregator");
 //   wifiMac.SetMpduAggregatorForAc (AC_BE,"ns3::MpduStandardAggregator", "MaxAmpduSize", UintegerValue (10240));


  // wifiMac.SetMsduAggregatorForAc (AC_VI, "ns3::MsduStandardAggregator", "MaxAmsduSize", UintegerValue (7935));
 // wifiMac.SetMsduAggregatorForAc (AC_VO, "ns3::MsduStandardAggregator", "MaxAmsduSize", UintegerValue (7935));
   //  wifiMac.SetMsduAggregatorForAc (AC_BE, "ns3::MsduStandardAggregator", "MaxAmsduSize", UintegerValue (3839));


    wifiMac.SetType ("ns3::StaWifiMac", "Ssid", SsidValue (ssid),"ActiveProbing", BooleanValue (false));
    NetDeviceContainer staDevice = wifi.Install (phy, wifiMac, staNodes);
    /* Setting mobility model */

    MobilityHelper mobility;
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();

    positionAlloc->Add (Vector (0.0, 0.0, 0.0));
    positionAlloc->Add (Vector (20, 0.0, 0.0));
    mobility.SetPositionAllocator (positionAlloc);

    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

    mobility.Install (staNodes.Get(0));
    mobility.Install (apNodes.Get(0));

    //install internet stack and addresses of nodes
    InternetStackHelper istack;
    istack.Install (apNodes);
    istack.Install (staNodes);

    Ipv4AddressHelper address;
    address.SetBase ("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer j =address.Assign (apDevice);
    Ipv4InterfaceContainer i =address.Assign (staDevice);
/*
    //energy model. BasicEnergySource+WifiRadioEnergyModel
    BasicEnergySourceHelper basicSourceHelper;
    basicSourceHelper.Set ("BasicEnergySourceInitialEnergyJ", DoubleValue (100));
    EnergySourceContainer s1 = basicSourceHelper.Install (apNodes);
    EnergySourceContainer s2 = basicSourceHelper.Install (staNodes);

    WifiRadioEnergyModelHelper radioEnergyHelper;
    radioEnergyHelper.Set ("TxCurrentA", DoubleValue (0.0857));
    radioEnergyHelper.Set ("RxCurrentA", DoubleValue (0.0528));
    radioEnergyHelper.Set ("IdleCurrentA", DoubleValue (0.0188));

    DeviceEnergyModelContainer d1 = radioEnergyHelper.Install (apDevice, s1);
    DeviceEnergyModelContainer d2 = radioEnergyHelper.Install (staDevice, s2);
*/
    //install on-off application to odd nodes



    InetSocketAddress remote = InetSocketAddress (j.GetAddress (0, 0), 80);











    OnOffHelper onh2("ns3::UdpSocketFactory", remote);
    onh2.SetAttribute ("PacketSize",  UintegerValue (psize));
    onh2.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
    onh2.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
    onh2.SetAttribute ("DataRate",DataRateValue (DataRate ("65Mbps")));
    ApplicationContainer source_apps2 =onh2.Install(staNodes.Get (0));
    source_apps2.Start (Seconds (0));
    source_apps2.Stop (Seconds (30.0));


    Ptr<OnOffApplication> onoffapp2;
     onoffapp2 = DynamicCast<OnOffApplication>(source_apps2.Get(0));
    onoffapp2->TraceConnectWithoutContext("Tx",  MakeBoundCallback (&TagMarker, AC_BK));

/*

     OnOffHelper onh4("ns3::UdpSocketFactory", remote);
     onh4.SetAttribute ("PacketSize",  UintegerValue (psize));
     onh4.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
     onh4.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
     onh4.SetAttribute ("DataRate",DataRateValue (DataRate ("1Mbps")));
     ApplicationContainer source_apps4 =onh4.Install(staNodes.Get (0));
     source_apps4.Start (Seconds (0));
     source_apps4.Stop (Seconds (30.0));


     Ptr<OnOffApplication> onoffapp4;
      onoffapp4 = DynamicCast<OnOffApplication>(source_apps4.Get(0));
      onoffapp4->TraceConnectWithoutContext("Tx",  MakeBoundCallback (&TagMarker, AC_VO));


     OnOffHelper onh3("ns3::UdpSocketFactory", remote);
     onh3.SetAttribute ("PacketSize",  UintegerValue (psize));
     onh3.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
     onh3.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
     onh3.SetAttribute ("DataRate",DataRateValue (DataRate ("24Mbps")));
     ApplicationContainer source_apps3 =onh3.Install(staNodes.Get (0));
     source_apps3.Start (Seconds (0));
     source_apps3.Stop (Seconds (30.0));


     Ptr<OnOffApplication> onoffapp3;
      onoffapp3 = DynamicCast<OnOffApplication>(source_apps3.Get(0));
      onoffapp3->TraceConnectWithoutContext("Tx",  MakeBoundCallback (&TagMarker, AC_BK));




      OnOffHelper onh("ns3::UdpSocketFactory", remote);
      onh.SetAttribute ("PacketSize",  UintegerValue (psize));
      onh.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
      onh.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
      onh.SetAttribute ("DataRate",DataRateValue (DataRate ("24Mbps")));
      ApplicationContainer source_apps =onh.Install(staNodes.Get (0));
      source_apps.Start (Seconds (0));
      source_apps.Stop (Seconds (30.0));





*/





      InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 80);
      PacketSinkHelper recvSink("ns3::UdpSocketFactory", local);
      sinkApp = recvSink.Install(apNodes.Get (0));
       sinkApp.Start (Seconds (0.0));
       sinkApp.Stop (Seconds (30.0));


     Config::ConnectWithoutContext("/NodeList/0/ApplicationList/0/$ns3::PacketSink/Rx",MakeCallback (&ReceivePacket));


    Config::ConnectWithoutContext("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyRxEnd", MakeCallback(&PhyRx));
    Config::ConnectWithoutContext("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/MacRx", MakeCallback(&MacRx));

    Simulator::Stop (Seconds (30.0));
    Simulator::Run ();


     NS_LOG_UNCOND ("Total Average Throughput Mac:" << MacRxSum*8/30000000 ) ;
     NS_LOG_UNCOND ("Total Average Throughput phy:" << PhyRxSum*8/30000000) ;
     NS_LOG_UNCOND ("Total Average Throughput :" << (totalRxBytesCounter*8)/30000000) ;



    Simulator::Destroy ();
    return 0;
}
