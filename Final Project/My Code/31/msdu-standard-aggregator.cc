/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 MIRKO BANCHI
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
 * Author: Mirko Banchi <mk.banchi@gmail.com>
 */

#include "ns3/log.h"
#include "ns3/uinteger.h"
#include "amsdu-subframe-header.h"
#include "msdu-standard-aggregator.h"
#include "ampdu-subframe-header.h"
#include "mpdu-standard-aggregator.h"

namespace ns3 {
double count =0 ;
uint32_t motevaset1=0;
bool change=false;
NS_LOG_COMPONENT_DEFINE ("MsduStandardAggregator");

NS_OBJECT_ENSURE_REGISTERED (MsduStandardAggregator);

TypeId
MsduStandardAggregator::GetTypeId (void)
{
    std::cout<<"msdu "<<std::endl;

  static TypeId tid = TypeId ("ns3::MsduStandardAggregator")
    .SetParent<MsduAggregator> ()
    .SetGroupName ("Wifi")
    .AddConstructor<MsduStandardAggregator> ()
    .AddAttribute ("MaxAmsduSize", "Max length in byte of an A-MSDU",
                   UintegerValue (7935),
                   MakeUintegerAccessor (&MsduStandardAggregator::m_maxAmsduLength),
                   MakeUintegerChecker<uint32_t> ())
  ;
  return tid;
}

MsduStandardAggregator::MsduStandardAggregator ()
{
}

MsduStandardAggregator::~MsduStandardAggregator ()
{
}

bool
MsduStandardAggregator::Aggregate (Ptr<const Packet> packet, Ptr<Packet> aggregatedPacket,
                                   Mac48Address src, Mac48Address dest)
{

if (change==false){
  NS_LOG_FUNCTION (this);
  Ptr<Packet> currentPacket;
  AmsduSubframeHeader currentHdr;

  uint32_t padding = CalculatePadding (aggregatedPacket);
  uint32_t actualSize = aggregatedPacket->GetSize ();

  if ((14 + packet->GetSize () + actualSize + padding) <= m_maxAmsduLength)
    {
      if (padding)
        {
          Ptr<Packet> pad = Create<Packet> (padding);
          aggregatedPacket->AddAtEnd (pad);
        }
      currentHdr.SetDestinationAddr (dest);
      currentHdr.SetSourceAddr (src);
      currentHdr.SetLength (packet->GetSize ());
      currentPacket = packet->Copy ();

      currentPacket->AddHeader (currentHdr);
      aggregatedPacket->AddAtEnd (currentPacket);
      std::cout<<"msdu "<<14 + packet->GetSize () + actualSize + padding<<std::endl;
      return true;
    }
  std::cout<<"msdu "<<"false"<<std::endl;
change=true;
return true;
}
 else {
    NS_LOG_FUNCTION (this);
    Ptr<Packet> currentPacket;
    AmpduSubframeHeader currentHdr;

    uint32_t padding = CalculatePadding (aggregatedPacket);
    uint32_t actualSize = aggregatedPacket->GetSize ();

    if ((4 + packet->GetSize () + actualSize + padding) <= 10240)
      {
        if (padding)
          {
            Ptr<Packet> pad = Create<Packet> (padding);
            aggregatedPacket->AddAtEnd (pad);
          }
        currentHdr.SetCrc (1);
        currentHdr.SetSig ();
        currentHdr.SetLength (packet->GetSize ());
        currentPacket = packet->Copy ();

        currentPacket->AddHeader (currentHdr);
        aggregatedPacket->AddAtEnd (currentPacket);
        count++;

        std::cout<<"mpdu " <<count*40 <<std::endl;

        return true;
      }
    change=false;
    return false;



  }


}




uint32_t
MsduStandardAggregator::CalculatePadding (Ptr<const Packet> packet)
{


  return (4 - (packet->GetSize () % 4 )) % 4;
}

} //namespace ns3
