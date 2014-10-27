/**
 * Project: Co-MATLAB-OMNeT
 * File name: BasicCosimNode.h
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

#ifndef __COSIM_BASIC_COSIM_NODE_H
#define __COSIM_BASIC_COSIM_NODE_H

#include "cosimScheduler.h"
#include "INETDefs.h"
#include "MACAddress.h"
#include <string>

/**
 *
 */
class BasicCosimNode : public cSimpleModule
{
public:

    static void sendOut(cSimpleModule* mod, cGate* outGate, cDatarateChannel* chOut, NetPkt* n);

  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void finish();

  private:
    cQueue* queue;
    cMessage* endTX;
    cGate* out;
    cChannel* chOut;
    cChannel* chIn;

    std::string linkType;

    cosimScheduler* cosimSched;
};

#endif


