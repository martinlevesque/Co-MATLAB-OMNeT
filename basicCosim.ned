//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

import FiWi.cosim.BasicCosimNode;
import inet.nodes.ethernet.EtherSwitch;
import inet.base.MMapBoard;
import inet.examples.ethernet.lans.MediumLAN;
import inet.world.ChannelControlExtended;
import inet.linklayer.ethernet.EtherBus;
import inet.experimental.linklayer.ieee80211.Ieee80211NicMesh;
import inet.linklayer.ethernet.EtherHub;
import inet.world.annotations.AnnotationManager;
import ned.DatarateChannel;
import inet.applications.dhcp.DHCPServer;
import inet.linklayer.ethernet.EthernetInterface;

network basicCosim
{
    parameters:


        @display("bgb=583,558;bgl=2");
    types:
        channel ZigBeeLink extends ned.DatarateChannel
        {
            datarate = 250kbps;
            delay = 0.0000001666666s + 0.002912s;
            ber = 0;
        }
    submodules:
        DRR3: BasicCosimNode {
            @display("p=266,194");
            linkType = "simplex";
        }
        HAN3: BasicCosimNode {
            @display("p=266,321");
            linkType = "simplex";
        }
    connections allowunconnected:

		//////////////////////
		// TDM channels

        // splitter <-> OLT

        HAN3.traf <--> ZigBeeLink <--> DRR3.traf;
}
