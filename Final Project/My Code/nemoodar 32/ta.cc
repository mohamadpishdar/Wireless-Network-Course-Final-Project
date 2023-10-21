
#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/dca-txop.h" 

 
using namespace ns3;
// declare For Calculate Cw
uint32_t cwold=15;
uint32_t countcollision=0;
ApplicationContainer sinkApps;
std::ofstream myfile ("p.txt",std::ofstream::binary);
uint32_t totalRxBytesCounter = 0;
// Add All Cw
uint32_t cw_all=0;
uint32_t count=0;

ApplicationContainer sinkApp;
 NodeContainer wifiStaNodes;

// Calculate Colition and Avrage Cw




void ReceivePacket (Ptr<const Packet> packet, const Address &)
{
  NS_LOG_UNCOND ("Received one packet!");

      count++;


    totalRxBytesCounter += packet->GetSize ();
        totalRxBytesCounter+= 1;


 NS_LOG_UNCOND ("count basteha daryaft shode " << count);
 NS_LOG_UNCOND ("Average Delay "<<count/(Simulator::Now ().GetSeconds()*1000));
 NS_LOG_UNCOND ("count basteha daryaft shode " <<Simulator::Now ().GetSeconds());

 //NS_LOG_UNCOND("th " << Simulator::Now ().GetSeconds()<< "\t"<<((totalRxBytesCounter*8.0)/1000000)/Simulator::Now ().GetSeconds());
 //NS_LOG_UNCOND("average packet transmition time " << Simulator::Now ().GetSeconds()<< "\t"<<((Simulator::Now ().GetSeconds())*1000)/count);


}




int 
main (int argc, char *argv[])
{
   
 // bool verbose = true;
  // Number of Server And Wifi
  uint32_t nserver = 1;
  uint32_t nWifi = 1;





// Definition server node
  NodeContainer serverNodes;
  serverNodes.Create (nserver);
  //CsmaHelper serverh;
 //serverh.SetChannelAttribute ("DataRate", StringValue ("10Mbps"));
  //serverh.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560)));

 // NetDeviceContainer serverDevices;
 // serverDevices = serverh.Install (serverNodes);

  // Defiition Of Wifi Nodes

  wifiStaNodes.Create (nWifi);
  NodeContainer wifiApNode =serverNodes.Get(0);

  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetChannel (channel.Create ());

  //The SetRemoteStationManager method tells the helper the type of rate control algorithm to use. Here, it is asking the helper to use the AARF algorithm
  WifiHelper wifi = WifiHelper::Default ();
  wifi.SetRemoteStationManager ("ns3::CaraWifiManager");
  wifi.SetStandard (WIFI_PHY_STANDARD_80211g);
 // wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
   //                             "DataMode", StringValue ("OfdmRate2Mbps"));

// wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager");
  NqosWifiMacHelper mac = NqosWifiMacHelper::Default ();

//creates an 802.11 service set identifier (SSID) object that will be used to set the value of the “Ssid” Attribute of the MAC layer implementation.
  //The particular kind of MAC layer is specified by Attribute as being of the "ns3::NqstaWifiMac" type.
  //This means that the MAC will use a “non-QoS station” (nqsta) state machine. Finally, the “ActiveProbing” Attribute is set to false.
  Ssid ssid = Ssid ("ns-3-ssid");
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (false));

  // install Wifi on Wifistatnodes
  NetDeviceContainer staDevices;
  staDevices = wifi.Install (phy, mac, wifiStaNodes);

  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));

  NetDeviceContainer apDevices;
  apDevices = wifi.Install (phy, mac, wifiApNode);

  /* Setting mobility model */
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();

  positionAlloc->Add (Vector (0.0, 0.0, 0.0));
  positionAlloc->Add (Vector (150, 0.0, 0.0));
  mobility.SetPositionAllocator (positionAlloc);

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  mobility.Install (wifiStaNodes.Get(0));
  mobility.Install (wifiApNode.Get(0));


  // Define and install Internet Stack On All Nodes
  InternetStackHelper stack;
  //stack.Install (serverNodes);
  stack.Install (wifiApNode);
  stack.Install (wifiStaNodes);

// Define Ip Address

  Ipv4AddressHelper address;



  address.SetBase ("10.1.3.0", "255.255.255.0");
address.Assign (staDevices);
   Ipv4InterfaceContainer i=address.Assign (apDevices);



  InetSocketAddress remote = InetSocketAddress (i.GetAddress (0, 0), 80);
  OnOffHelper onh("ns3::UdpSocketFactory", remote);
  onh.SetAttribute ("PacketSize",  UintegerValue (200));
 onh.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  onh.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  onh.SetAttribute ("DataRate",DataRateValue (DataRate ("10Mbps")));




  InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 80);
  PacketSinkHelper recvSink("ns3::UdpSocketFactory", local);
  sinkApp = recvSink.Install(serverNodes.Get (0));
   sinkApp.Start (Seconds (0.0));
   sinkApp.Stop (Seconds (100.0));





// install Source app on all wifi nodes
ApplicationContainer clientApps ;

    clientApps =  onh.Install (wifiStaNodes.Get (0));
    clientApps.Start (Seconds (0.0));
    clientApps.Stop (Seconds (100.0));



 // Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  // scheduling For All Wifi Nodes

  Config::ConnectWithoutContext("/NodeList/0/ApplicationList/0/$ns3::PacketSink/Rx",MakeCallback (&ReceivePacket));


  Simulator::Stop (Seconds (20.0));

  Simulator::Run ();
  Simulator::Destroy ();
//std::cout<<cw_all<<std::endl;
//std::cout<<cw_count<<std::endl;
//std::cout<<"-----------"<<std::endl;
//long avg=cw_all/cw_count;
//std::cout<<avg<<std::endl;
  return 0;
}
