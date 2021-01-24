//gsm
#include "ns3/lte-helper.h"
#include "ns3/epc-helper.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/config-store.h"
//#include "ns3/gtk-config-store.h"
//.............................................................................................................
using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("EpcFirstExample");
int
main (int argc, char *argv[])
{
uint16_t numberOfNodes = 2; // numberOfNodes = 6 for CDMA
 double simTime = 1.1;
 double distance = 60.0;
 double interPacketInterval = 100;
 // Command line arguments
 CommandLine cmd;
 cmd.Parse(argc, argv);
 Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
//This will instantiate some common objects (e.g., the Channel object) and provide the methods to add eNBs and UEs and configure them.
 Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper> ();
//PointToPointEpcHelper, which implements an EPC based on point-to-point links.
//EpcHelper will also automatically create the PGW node and configure it so that it can properly handletraffic from/to the LTE radio access network.
 lteHelper->SetEpcHelper (epcHelper);
//Then, you need to tell the LTE helper that the EPC will be used:
 ConfigStore inputConfig;
 inputConfig.ConfigureDefaults();
//Specify configuration parameters of the objects that are being used for the simulation
 // parse again so you can override default values from the command line
 cmd.Parse(argc, argv);
 Ptr<Node> pgw = epcHelper->GetPgwNode ();
//EpcHelper will also automatically create the PGW node and configure it so that it can properly handle traffic from/to the LTE radio access network.
 // Create a single RemoteHost
 NodeContainer remoteHostContainer;
 remoteHostContainer.Create (1);
 Ptr<Node> remoteHost = remoteHostContainer.Get (0);
 InternetStackHelper internet;
 internet.Install (remoteHostContainer);
 // Create the Internet
 PointToPointHelper p2ph;
 p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
 p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
 p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
 NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
 Ipv4AddressHelper ipv4h;
 ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
 Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
 // interface 0 is localhost, 1 is the p2p device
 Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);
 Ipv4StaticRoutingHelper ipv4RoutingHelper;
 Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
 remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);
 NodeContainer ueNodes;
 NodeContainer enbNodes;
 enbNodes.Create(numberOfNodes);
 ueNodes.Create(numberOfNodes);
 // Install Mobility Model
 Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
 for (uint16_t i = 0; i < numberOfNodes; i++)
 {
 positionAlloc->Add (Vector(distance * i, 100, 100));
 }
 MobilityHelper mobility;
 mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
 mobility.SetPositionAllocator(positionAlloc);
 mobility.Install(enbNodes);
 mobility.Install(ueNodes);
 // Install LTE Devices to the nodes
 NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice (enbNodes);
 NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice (ueNodes);
 // Install the IP stack on the UEs
 internet.Install (ueNodes);
 Ipv4InterfaceContainer ueIpIface;
 ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevs));
 // Assign IP address to UEs, and install applications
 for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
 {
 Ptr<Node> ueNode = ueNodes.Get (u);
 // Set the default gateway for the UE
 Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
 ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
 }
 // Attach one UE per eNodeB
 for (uint16_t i = 0; i < numberOfNodes; i++)
 {
 lteHelper->Attach (ueLteDevs.Get(i), enbLteDevs.Get(i));
 // side effect: the default EPS bearer will be activated
 }
 // Install and start applications on UEs and remote host
 uint16_t dlPort = 1234;
 uint16_t ulPort = 2000;
 uint16_t otherPort = 3000;
 ApplicationContainer clientApps;
 ApplicationContainer serverApps;
 for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
 {
 ++ulPort;
 ++otherPort;
 PacketSinkHelper dlPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress
(Ipv4Address::GetAny (), dlPort));
 PacketSinkHelper ulPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress
(Ipv4Address::GetAny (), ulPort));
 PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress
(Ipv4Address::GetAny (), otherPort));
 serverApps.Add (dlPacketSinkHelper.Install (ueNodes.Get(u)));
 serverApps.Add (ulPacketSinkHelper.Install (remoteHost));
 serverApps.Add (packetSinkHelper.Install (ueNodes.Get(u)));
 UdpClientHelper dlClient (ueIpIface.GetAddress (u), dlPort);
 dlClient.SetAttribute ("Interval", TimeValue (MilliSeconds(interPacketInterval)));
 dlClient.SetAttribute ("MaxPackets", UintegerValue(1000000));
 UdpClientHelper ulClient (remoteHostAddr, ulPort);
 ulClient.SetAttribute ("Interval", TimeValue (MilliSeconds(interPacketInterval)));
 ulClient.SetAttribute ("MaxPackets", UintegerValue(1000000));
 UdpClientHelper client (ueIpIface.GetAddress (u), otherPort);
 client.SetAttribute ("Interval", TimeValue (MilliSeconds(interPacketInterval)));
 client.SetAttribute ("MaxPackets", UintegerValue(1000000));
 clientApps.Add (dlClient.Install (remoteHost));
 clientApps.Add (ulClient.Install (ueNodes.Get(u)));
 if (u+1 < ueNodes.GetN ())
 {
 clientApps.Add (client.Install (ueNodes.Get(u+1)));
 }
 else
 {
 clientApps.Add (client.Install (ueNodes.Get(0)));
 }
 }
 serverApps.Start (Seconds (0.01));
 clientApps.Start (Seconds (0.01));
 lteHelper->EnableTraces ();
 // Uncomment to enable PCAP tracing
 AsciiTraceHelper ascii;
 p2ph.EnableAsciiAll(ascii.CreateFileStream("lab5.tr"));
 p2ph.EnablePcapAll("lena-epc-first");
 Simulator::Stop(Seconds(simTime));
 Simulator::Run();
 /*GtkConfigStore config;
 config.ConfigureAttributes();*/
 Simulator::Destroy();
 return 0;
}
