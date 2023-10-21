
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/olsr-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/applications-module.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

NS_LOG_COMPONENT_DEFINE ("WifiSimpleAdhocGrid");

using namespace ns3;
uint32_t count =0;
std::ofstream myfile ("project.txt",std::ofstream::binary);
uint32_t average =0;
 NodeContainer c;
 uint32_t totalRxBytesCounter = 0;

 ApplicationContainer sinkApp;



void ReceivePacket (Ptr<const Packet> packet, const Address &)
{
  NS_LOG_UNCOND ("Received one packet!");
  QosTag q_tag;
  if (packet->PeekPacketTag(q_tag))
  {
      NS_LOG_UNCOND ("Packet Tag value " << (int)q_tag.GetTid());
  }
  count++;
    totalRxBytesCounter += packet->GetSize ();
        totalRxBytesCounter+= 1;
if (count==1000)
    std::cout<<"time "<<Simulator::Now ().GetSeconds();


 NS_LOG_UNCOND ("count basteha daryaft shode " << count);
 NS_LOG_UNCOND("th " << Simulator::Now ().GetSeconds()<< "\t"<<((totalRxBytesCounter*8.0)/1000000)/Simulator::Now ().GetSeconds());
 NS_LOG_UNCOND("average packet transmition time " << Simulator::Now ().GetSeconds()<< "\t"<<((Simulator::Now ().GetSeconds())*1000)/count);


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


  std::string phyMode ("DsssRate1Mbps");
  //uint32_t packetSize = 2264; // bytes
  uint32_t numNodes = 2;  // by default, 5x5
  uint32_t sinkNode = 0;
  uint32_t sourceNode = 1;


  // disable fragmentation for frames below 2200 bytes
 // Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("1500"));
  // turn off RTS/CTS for frames below 2200 bytes
// Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2300"));
  // Fix non-unicast data rate to be the same as that of unicast
 // Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue (phyMode));


  c.Create (numNodes);

  // The below set of helpers will help us to put together the wifi NICs we want
  WifiHelper wifi=WifiHelper::Default ();


  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
  // set it to zero; otherwise, gain will be added
 // wifiPhy.Set ("RxGain", DoubleValue (-10) );
  // ns-3 supports RadioTap and Prism tracing extensions for 802.11b
  //wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);

  YansWifiChannelHelper wifiChannel=YansWifiChannelHelper::Default ();
//wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
// wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
//wifiChannel.AddPropagationLoss("ns3::LogDistancePropagationLossModel","Exponent", DoubleValue (40));
wifiPhy.SetChannel (wifiChannel.Create ());

  // Add a QoS upper mac, and disable rate control
  QosWifiMacHelper wifiMac = QosWifiMacHelper::Default ();
  wifi.SetStandard (WIFI_PHY_STANDARD_80211g);

wifi.SetRemoteStationManager ("ns3::IdealWifiManager");
// wifi.SetRemoteStationManager ("ns3::IdealWifiManager", "RtsCtsThreshold", UintegerValue (1000));
  Ssid ssid = Ssid ("ns-3-ssid");
  wifiMac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (false));
  //wifiMac.SetMsduAggregatorForAc (AC_BK, "ns3::MsduStandardAggregator", "MaxAmsduSize", UintegerValue (3839));
 // wifiMac.SetMsduAggregatorForAc (AC_VO, "ns3::MsduStandardAggregator", "MaxAmsduSize", UintegerValue (7935));
  //wifiMac.SetMsduAggregatorForAc (AC_VI, "ns3::MsduStandardAggregator", "MaxAmsduSize", UintegerValue (7935));

  // install Wifi on Wifistatnodes
  NetDeviceContainer staDevices;
  staDevices = wifi.Install (wifiPhy, wifiMac,  c.Get(1));

  wifiMac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));

  NetDeviceContainer apDevices;
  apDevices = wifi.Install (wifiPhy, wifiMac, c.Get(0));


  /* Setting mobility model */
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();

  positionAlloc->Add (Vector (0.0, 0.0, 0.0));
  positionAlloc->Add (Vector (120, 0.0, 0.0));
  mobility.SetPositionAllocator (positionAlloc);

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  mobility.Install (c.Get(0));
  mobility.Install (c.Get(1));

  InternetStackHelper internet;
  internet.Install (c);

  Ipv4AddressHelper ipv4;
  NS_LOG_INFO ("Assign IP Addresses.");
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  ipv4.Assign(staDevices);
  Ipv4InterfaceContainer i = ipv4.Assign (apDevices);


  InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 80);
  PacketSinkHelper recvSink("ns3::UdpSocketFactory", local);
  sinkApp = recvSink.Install(c.Get (sinkNode));
   sinkApp.Start (Seconds (0.0));
   sinkApp.Stop (Seconds (30.0));


  InetSocketAddress remote = InetSocketAddress (i.GetAddress (sinkNode, 0), 80);
  OnOffHelper onh("ns3::UdpSocketFactory", remote);
  onh.SetAttribute ("PacketSize",  UintegerValue (2264));
  onh.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  onh.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  onh.SetAttribute ("DataRate",DataRateValue (DataRate ("24Mbps")));
  ApplicationContainer source_apps =onh.Install(c.Get (sourceNode));
  source_apps.Start (Seconds (0.5));
  source_apps.Stop (Seconds (30.0));

  Ptr<OnOffApplication> onoffapp;
   onoffapp = DynamicCast<OnOffApplication>(source_apps.Get(0));
   onoffapp->TraceConnectWithoutContext("Tx",  MakeBoundCallback (&TagMarker, AC_VO));


  OnOffHelper onh2("ns3::UdpSocketFactory", remote);
  onh.SetAttribute ("PacketSize",  UintegerValue (2264));
  onh.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  onh.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  onh.SetAttribute ("DataRate",DataRateValue (DataRate ("24Mbps")));
  ApplicationContainer source_apps2 =onh2.Install(c.Get (sourceNode));
  source_apps2.Start (Seconds (0.5));
  source_apps2.Stop (Seconds (30.0));


  Ptr<OnOffApplication> onoffapp2;
   onoffapp2 = DynamicCast<OnOffApplication>(source_apps2.Get(0));
   onoffapp2->TraceConnectWithoutContext("Tx",  MakeBoundCallback (&TagMarker, AC_VI));


   OnOffHelper onh3("ns3::UdpSocketFactory", remote);
   onh.SetAttribute ("PacketSize",  UintegerValue (2264));
   onh.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
   onh.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
   onh.SetAttribute ("DataRate",DataRateValue (DataRate ("24Mbps")));
   ApplicationContainer source_apps3 =onh3.Install(c.Get (sourceNode));
   source_apps3.Start (Seconds (0.5));
   source_apps3.Stop (Seconds (30.0));


   Ptr<OnOffApplication> onoffapp3;
    onoffapp3 = DynamicCast<OnOffApplication>(source_apps3.Get(0));
    onoffapp3->TraceConnectWithoutContext("Tx",  MakeBoundCallback (&TagMarker, AC_BK));


  Config::ConnectWithoutContext("/NodeList/0/ApplicationList/0/$ns3::PacketSink/Rx",MakeCallback (&ReceivePacket));





  Simulator::Stop (Seconds (30.0));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
   }


