/**
 * Project: Co-MATLAB-OMNeT
 * File name: BasicCosimNode.cc
 *
 *
 * @author Martin Levesque
 * @version v 1.0
 *
 * @see The GNU Public License (GPL)
 */

/**

 Co-MATLAB-OMNeT is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 HLA-OMNeT++ is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with HLA-OMNeT++.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "BasicCosimNode.h"
#include <omnetpp.h>

#include "netpkt_m.h"
#include "MyUtil.h"
#include <math.h>

using namespace std;

Define_Module (BasicCosimNode);

void BasicCosimNode::initialize()
{
    cout << "BasicCosimNode::initialize " << endl;

    linkType = par("linkType").stringValue();

    cout << getName() << " link type = " << linkType << endl;

    queue = new cQueue("Node Q");
    endTX = new cMessage("tx");

    out = gate("traf$o");

    chIn = NULL;
    chOut = NULL;

    if (out && out->isConnected())
    {
        chOut = out->getTransmissionChannel();

        cout << "ch out " << chOut << endl;

        if (linkType == "duplex")
        {
            cModule* tmpAdj = MyUtil::getNeighbourOnGate(this, "traf$o");

            if (tmpAdj)
            {
                cGate* gateAdj = tmpAdj->gate("externTraf$o");

                if ( ! gateAdj || ! gateAdj->isConnected())
                {
                    gateAdj = tmpAdj->gate("traf$o");
                }

                if (gateAdj)
                {
                    chIn = gateAdj->getTransmissionChannel();
                }
            }
        }
    }

    bool res2 = (linkType == "simplex" && chOut != NULL);

    cout << "res 2 " << res2 << endl;

    ASSERT((linkType == "duplex" && chOut && chIn) || (linkType == "simplex" && chOut));

    cosimSched = dynamic_cast<cosimScheduler*>(simulation.getScheduler());
}

void BasicCosimNode::sendOut(cSimpleModule* mod, cGate* outGate, cDatarateChannel* chOut, NetPkt* n)
{
    ASSERT(n);
    ASSERT(mod);
    ASSERT(outGate);
    ASSERT(chOut);

    int nbZigBeeFramesRequired = ceil((double)n->getByteLength() / (double)114); // 114 bytes is the maximum payload with 2 bytes addressing

    cout << "nbZigBeeFramesRequired = " << nbZigBeeFramesRequired << endl;

    double delay = 0.0000001666666 + (double)nbZigBeeFramesRequired * 0.002912;

    cout << "delay = " << delay << endl;
    cout << "frame size = " << n->getBitLength() << endl;

    chOut->setDelay(delay);
    mod->send(n, outGate);
}

void BasicCosimNode::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage() && ! dynamic_cast<NetPkt*>(msg))
    {
        cout << "BasicCosimNode::handleMessage 1 " << endl;
        if (msg == endTX)
        {
            if ((chIn && chIn->isBusy()) || chOut->isBusy())
            {
                scheduleAt((chIn && chIn->isBusy()) ? chIn->getTransmissionFinishTime() : chOut->getTransmissionFinishTime(), endTX);
            }
            else
            {
                cMessage* n = (cMessage*)queue->pop();
                sendOut(this, out, dynamic_cast<cDatarateChannel*>(chOut), dynamic_cast<NetPkt*>(n));

                if ( ! queue->empty())
                {
                    scheduleAt(chOut->getTransmissionFinishTime(), endTX);
                }
            }
        }
        cout << "BasicCosimNode::handleMessage 2 " << endl;
    }
    else
    if (msg->isSelfMessage() && dynamic_cast<NetPkt*>(msg)) // New packet newly create
    {
        cout << "BasicCosimNode::handleMessage 3 " << getName() << " chIn " << (chIn == NULL) << " ch out " << (chOut == NULL) << endl;
        if ((chIn && chIn->isBusy()) || chOut->isBusy())
        {
            cout << "wtf 1" << endl;
            queue->insert(msg);
            cout << "wtf 2" << endl;
            if ( ! endTX->isScheduled())
            {
                cout << "wtf 3" << endl;
                scheduleAt((chIn && chIn->isBusy()) ? chIn->getTransmissionFinishTime() : chOut->getTransmissionFinishTime(), endTX);
                cout << "wtf 4" << endl;
            }
        }
        else
        {
            cout << "wtf 5" << endl;
            sendOut(this, out, dynamic_cast<cDatarateChannel*>(chOut), dynamic_cast<NetPkt*>(msg));
            cout << "wtf 6" << endl;
        }
    }
    else
    {
        NetPkt* p = dynamic_cast<NetPkt*>(msg);

        cout << "FROM traf !!! " << getName() << " id " << p->getId() << " received at " << simTime().dbl()  << endl;

        if (string(p->getDestAddress()) == string(getName()))
        {
            cout << "BasicCosimNode dest 1" << endl;
            if (cosimSched)
            {
                cout << "BasicCosimNode dest 2" << endl;
                cosimSched->sendResponseToRTI(p);
            }
        }
        else
        {
            error("BasicCosimNode::handleMessage - unhandled scenario !!!");
        }

    }

}



void BasicCosimNode::finish()
{

    delete queue;
}

