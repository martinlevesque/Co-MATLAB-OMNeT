/**
 * Project: Co-MATLAB-OMNeT
 * File name: cosimScheduler.h
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

#ifndef CO_SIM_SCHEDULER_H_
#define CO_SIM_SCHEDULER_H_

#include "platdep/sockets.h"
#include "platdep/timeutil.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/wait.h>
#include <omnetpp.h>
#include "netpkt_m.h"
#include <pthread.h>
#include <string>

#define PRINT 1

class cosimScheduler : public cScheduler
{
public:
    //static cMessage* grantMsg;

    cosimScheduler();
    virtual ~cosimScheduler();

    timeval baseTime;
    virtual void startRun();
    virtual void endRun();
    virtual void executionResumed();
    virtual cMessage *getNextEvent();
    static void setInterfaceModule(cModule *module);
    virtual int sendResponseToRTI(cMessage *msg);

    void sendACKGrant(bool withSleep);

protected:
    std::string pendingReceivedMsg;
    time_t simulationBeginTime;
};

#endif /* CO_SIM_SCHEDULER_H_ */

