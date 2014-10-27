/**
 * Project: Co-MATLAB-OMNeT
 * File name: cosimScheduler.cc
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

#include "cosimScheduler.h"
#include <string>
#include <unistd.h>
#include <time.h>
#include "cMessage.h"
#include "MyUtil.h"
#include "netpkt_m.h"

using std::string;
using namespace std;

bool start;
bool socketEstablished;

//cModule* module;

int connectSocket;
int listenSocket;

double logicalTimeRTI;
bool sentRTIACK = true;

NetPkt* grantMsg = NULL;

int readn(int fd,char * buf, size_t count,int flag);
char *search(char *buff,char * end);

//Omnet packet for Network messages and Info messages (look at netpkt.msg and infomsg.pkt file for specific informations)
NetPkt *msgComRequestFromRTI;

int incr=1;
int incr2=1;

int MAX_MSG=50000;
int LINE_ARRAY_SIZE=(MAX_MSG+1);

Register_Class(cosimScheduler);


inline std::ostream& operator<<(std::ostream& os, const timeval& tv)
{
    return os << (unsigned long)tv.tv_sec << "s" << tv.tv_usec << "us";
}


/**
 * Sets the interface that will handle messages from external federates
 * \param Interface module between Oment and the Federation
 */
void cosimScheduler::setInterfaceModule(cModule *mod)
{
    if (!mod)
        throw new cRuntimeError("cosimScheduler: setInterfaceModule(): arguments must be not-NULL");
    //module = mod;
}

int ACKGrant(const string& pendingMsg)
{
    if ( ! sentRTIACK)
    {
        sentRTIACK = true;

        int bytesSent = 0;

        string msgToSend = pendingMsg;

        if (msgToSend == "")
        {
            msgToSend += "-\n";
        }
        else
        {
            msgToSend += "\n"; // there was no newline and we need one
        }

        send(connectSocket, msgToSend.c_str(), msgToSend.length(), 0);

        cout << "ACK GRANT - bytes sent = " << bytesSent << endl;



        return bytesSent;
    }

    return 0;
}

/**
 * Closes socket connection
 */
void closeConnection()
{
    int bSentGrant = ACKGrant("");

    cout << "Sent bytes msg for grant ? " << bSentGrant << endl;

    printf("Closing socket..\n");
    close(connectSocket);
    printf("Closing listen socket..\n");
    close(listenSocket);
    printf("patate\n");
}

/**
 * SIGINT handle
 */
void sigproc(int signo)
{
    printf("OMNET -- rti sched, sigproc\n");

    signal (SIGINT, sigproc);
    //module->getParentModule()->callFinish();
    pthread_exit(NULL);
}

/**
 * Creates a server to get and send messages to/from RTI
 */
void *listenRTI(void *)
{
    signal(SIGINT, sigproc);
    int i;
    int pid;
    unsigned short int listenPort = 25000;
    socklen_t clientAddressLength;
    struct sockaddr_in clientAddress, serverAddress;
    char line[LINE_ARRAY_SIZE];

    grantMsg = new NetPkt("test");

    // Create socket for listening for client connection requests.
    listenSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (listenSocket < 0) {
        printf("cannot create listen socket \n");
        exit(1);
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(listenPort);

    int yesReuseSocket = 1;

    // lose the pesky "Address already in use" error message
    if (setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &yesReuseSocket,sizeof(int)) == -1)
    {
        perror("setsockopt");
        exit(1);
    }

    if (bind(listenSocket,(struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
        printf("cannot bind socket \n");
        exit(1);
    }

    listen(listenSocket, 5);

    printf("Scheduler:*****************Connect To RTI*******************\n");
    memset(&clientAddress, 0x0, sizeof(clientAddress));
    memset(&connectSocket, 0x0, sizeof(connectSocket));
    memset(&clientAddressLength, 0x0, sizeof(clientAddressLength));
    clientAddressLength = sizeof(clientAddress);

    printf("SCHEDULER 2:*************** after con to RTI\n");

    connectSocket = accept(listenSocket,(struct sockaddr *) &clientAddress,&clientAddressLength);

    socketEstablished = true;

    printf("connectSocket accept:%d errno accept:%s\n",connectSocket,strerror(errno));
    if (connectSocket < 0) {
        printf("cannot accept connection \n");
    }

    int n_bytes=0;
    while (1)
    {
        memset(line, 0x0, LINE_ARRAY_SIZE);
        memset(line, 0x0, LINE_ARRAY_SIZE);

        printf("before read\n");

        if((n_bytes=readn(connectSocket,line, LINE_ARRAY_SIZE,1))>0){
            char *content=line;
            char * p = NULL;

            printf("read from the socket !! %s\n", content);

            //printf("RTI_SCHED, received %s\n", line);

            //EV << "READ this from the socket scheduler " << content << endl;
            //if (PRINT)


            // Get a request of simulation of a message from "source" to "destination"
            //<msgComRequestFromRTI id="1" source="tn_router" destination="ap_wa2" payload="100" time="14.299999999999965"/>
            if ((p=strstr(content,"END_SIMULATION"))!=NULL)
            {
                printf("OMNET - END_SIMULATION\n");

                simulation.callFinish();
                pthread_exit(NULL);
                break;
            }
            else
            if ((p=strstr(content,"msgComRequestFromRTI"))!=NULL)
            {
                printf("rrreeeaaaddd this from the sock ->%s<-\n", content);

                char name[LINE_ARRAY_SIZE];
                sprintf(name,"msgComRequestFromRTI_%d",incr);
                incr++;
                char *end=" ";
                
                //printf("COM REQUEST FROM RTI %s\n", content);

                p = strstr(content,"id");
                p += strlen("id")+2;
                char *id = search(p,end);

                p = strstr(content,"source");
                p += strlen("source")+2;
                char *srcAddr = search(p,end);

                p = strstr(content,"destination");
                p += strlen("destination")+2;
                char *destAddr = search(p,end);

                p = strstr(content,"payload");
                p += strlen("payload")+2;
                char *payload = search(p,end);

                //printf("powerSysNodeType = %s\n", powerSysNodeType);

                p = strstr(content,"time");
                p += strlen("time")+2;
                char *time = search(p,"/>");

                double fTime = atof(time);


                printf("time = %s\n", time);
                printf("REAL time = %.9f\n", fTime);

                msgComRequestFromRTI = new NetPkt(name);
                if (id!=NULL)
                    msgComRequestFromRTI->setId(id);
                if (srcAddr!=NULL)
                    msgComRequestFromRTI->setSrcAddress(srcAddr);
                if (destAddr!=NULL)
                    msgComRequestFromRTI->setDestAddress(destAddr);

                if (payload!=NULL)
                {
                    msgComRequestFromRTI->setPayload(payload);
                    msgComRequestFromRTI->setByteLength(string(payload).length());
                }

                cout << "LENGTH PKT " << msgComRequestFromRTI->getByteLength() << endl;

                msgComRequestFromRTI->setDestStatus("reachable");

                msgComRequestFromRTI->setTimestamp();

                cModule* modSource = dynamic_cast<cModule*>(simulation.findObject(srcAddr));

                if ( ! modSource)
                {
                    cout << "source addr " << srcAddr << endl;
                    perror("Mod source problem, cosim sched");
                    exit(1);
                }

                msgComRequestFromRTI->setArrival(modSource,-1);
                msgComRequestFromRTI->setSentFrom(modSource, -1, simulation.getSimTime());


                if (time!=NULL)
                {
                    msgComRequestFromRTI->setTimestamp(fTime);
                    msgComRequestFromRTI->setArrivalTime(fTime);
                }


                if (id!=NULL&&srcAddr!=NULL&&destAddr!=NULL&&payload!=NULL&&time!=NULL)
                {

                    simulation.msgQueue.insert(msgComRequestFromRTI);
                    simulation.msgQueue.sort();
                    free(id);
                    free(srcAddr);
                    free(destAddr);
                    free(payload);
                    free(time);
                    p = NULL;
                }
                else
                {
                    delete msgComRequestFromRTI;
                    perror("problem netpkt");
                    exit(1);
                }

                //if (PRINT)
                    printf("Scheduler:**********Request From federate at time:%f**********\n", logicalTimeRTI);

                //logicalTimeRTI = ceilf(fTime * 1000) / 1000;
                printf("REAL2 time = %.10f\n", logicalTimeRTI);

            }
            else if (strstr(content,"Start Omnet:")!=NULL)
            {
                printf("STARTING OMNET !!!!\n");

                start = true;
                if ((p = strstr(content,":"))!=NULL)
                {
                    p++;
                    //if (PRINT)
                        printf("Scheduler:**************Start Omnet at time:%f**************\n",logicalTimeRTI);
                }
                p = NULL;
            }
            else if (strstr(content,"Event Time Grant")!=NULL)
            {
                printf("EVENT TIME GRANT");

                if ((p = strstr(content,":"))!=NULL)
                {
                    p++;

                    logicalTimeRTI = atof(p);

                    printf("Scheduler:**************Event step at time:%.10f***************\n",logicalTimeRTI);
//                    if ((int)logicalTimeRTI%100==0)
                    //    printf("Event Time Grant float [%f]\n",atof(p));
                    p = NULL;
                }

                sentRTIACK = false;
            }
            p=NULL;
            content=NULL;
        }
        else if (n_bytes == 0)
        {
            printf("Connection closed\n");
            connectSocket = INVALID_SOCKET;
            // module->getParentModule()->callFinish();
            pthread_exit(NULL);
            break;
        }
        else if (n_bytes==SOCKET_ERROR)
        {
            printf("Connection Error\n");
            connectSocket = INVALID_SOCKET;
            n_bytes=0;
            simulation.callFinish();
            pthread_exit(NULL);
            break;
        }
    }
}

/**
 *  Constructor
 */
cosimScheduler::cosimScheduler() : cScheduler()
{
    listenSocket = INVALID_SOCKET;
    connectSocket = INVALID_SOCKET;
    pendingReceivedMsg = "";
}

/**
 * Destructor
 */
cosimScheduler::~cosimScheduler()
{
}

/**
 * Start execution
 */
void cosimScheduler::startRun()
{
    printf("OMNET -- rti sched, startRun\n");
    socketEstablished = false;
    start = false;
    //module = NULL;
    msgComRequestFromRTI = NULL;
    logicalTimeRTI = 0;

    int rc;
    pthread_t listenRtiThread;
    rc = pthread_create(&listenRtiThread, NULL, listenRTI, NULL);

    if (rc)
        printf("Thread: listenRtiThread creation failed: %d\n", rc);

    simulationBeginTime = time(NULL);
}

/**
 * End execution
 */
void cosimScheduler::endRun()
{
    static bool simStatsWrote = false;

    if ( ! simStatsWrote)
    {
        simStatsWrote = true;
        int64 eventNumber = simulation.getEventNumber();
        long secondsElapsed = time(NULL) - simulationBeginTime;
        long secsPerSimSec = secondsElapsed / (long)simTime().dbl();
        long long eventsPerSimSec = (long long)eventNumber / (long long)simTime().dbl();

        char stats[500];
        sprintf(stats, "%lld %ld %ld %ld %lld", (long long)eventNumber, secondsElapsed, (long)simTime().dbl(), secsPerSimSec, eventsPerSimSec);

        printf("OMNET -- End Run..events = %lld, events per sim sec = %lld, seconds elapsed: %ld, secsPerSimSec: %ld\n", (long long)eventNumber, eventsPerSimSec, secondsElapsed, secsPerSimSec);
        closeConnection();

    }
}

/**
 * Resume an execution
 */
void cosimScheduler::executionResumed()
{
    if (PRINT)
        printf("OMNET -- rti sched, executionResumed\n");

    gettimeofday(&baseTime, NULL);
    baseTime = timeval_substract(baseTime, SIMTIME_DBL(sim->getSimTime()));
}

void cosimScheduler::sendACKGrant(bool withSleep)
{
    if ( ! sentRTIACK)
    {
        ACKGrant(pendingReceivedMsg);
        pendingReceivedMsg = "";

        if (withSleep)
            usleep(500000);
    }
}

/**
* Get next event to schedule.
* \retval Returns next message
*/
cMessage *cosimScheduler::getNextEvent()
{
    //if (PRINT)
    //    printf("OMNET -- cosim sched, getNextEvent 1\n");

    // Need to wait for the socket to be established.
    while ( ! socketEstablished)
    {
        //printf("cosimScheduler::getNextEvent waiting for the socket to be established...\n");
        sleep(1);
    }

    cMessage *msg = NULL;

    while ((msg = simulation.msgQueue.peekFirst())==NULL)
    {
        // Starting of simulation
        if (start)
        {
            char tmp[LINE_ARRAY_SIZE];
            // Request to simulate the next message
            sprintf(tmp,"nextMessageRequest time:%f\r\n\r\n",logicalTimeRTI);
        }

        if (PRINT)
            printf("OMNET no msg to send...\n");

        sendACKGrant(true);
    }

    while ((msg = simulation.msgQueue.peekFirst())->getArrivalTime() > logicalTimeRTI)
    {
        if (PRINT)
            printf("Scheduler:Next Event at (%f) Request Advance from (%.10f) to (%.10f)\n",SIMTIME_DBL(msg->getArrivalTime()), logicalTimeRTI, SIMTIME_DBL(msg->getArrivalTime()));

        if (msg != NULL && PRINT)
            printf("Processing msg %s\n", msg->getFullName());

        sendACKGrant(true);
    }

    msg = simulation.msgQueue.peekFirst();

    if ( ! sentRTIACK && msg->getArrivalTime() >= logicalTimeRTI)
    {
        sendACKGrant(false);

        if (PRINT)
            printf("i have sent, cur time is %f, logical time = %f, time next msg %f\n", simulation.getSimTime().dbl(), logicalTimeRTI, simulation.msgQueue.peekFirst()->getArrivalTime().dbl() );
    }

    return msg;
}

/**
 * Send the status of a simulated message to the sender. The message will contain the id, source and destination,
 * status and total service time
 * \param msg Pointer to the message to send
 * \retval Return < 0 if it fails to send the message
 */
int cosimScheduler::sendResponseToRTI(cMessage *msg)
{
    NetPkt* responseMsg=check_and_cast<NetPkt *>(msg);

    string id = string(responseMsg->getId());
    string source=string(responseMsg->getSrcAddress());
    string destination=string(responseMsg->getDestAddress());
    string payload=string(responseMsg->getPayload());
    string destStatus = string(responseMsg->getDestStatus());
    string serviceTime = SIMTIME_STR(simTime() - responseMsg->getTimestamp());

    pendingReceivedMsg += id + "," + SIMTIME_STR(simTime()) + "," + serviceTime + ";";

    delete msg;
    return 0;
}

/**
 * Reads n characters
 */
int readn(int fd,char * buf, size_t count,int flag){

    int nleft=0, nread=0, tot=0;
    char *tmp = buf;

    if (flag==1) nleft = 1; else nleft = count;

    while (nleft>0)
    {
        if ((nread=read(fd,buf,nleft))<0)
        {
            if (errno==EINTR)
                continue;
            else
                return(nread);
        }
        else if (nread==0)
            break;

        if  (flag!=1) nleft-=nread;

        buf += nread;
        tot += nread;
        if (strstr(tmp,"\n")!=NULL)
        {
            tmp[tot-1]='\0';
            return (tot);
        }
    }

    return(tot);
}

/**
* brief Return the pointer of the end character in the string buff
* \param buff Input string
* \param end String to find
* \retval If it finds the string then it returns the pointer to such substring, otherwise it returns NULL
*/
char * search(char *buff, char *end)
{


    char *st=NULL;
    char *exp=NULL;

    st = strstr(buff,end);

    if (st!=NULL)
    {
        int dimReq=strlen(buff)-strlen(st);
        
        if (dimReq<1) return NULL;

        exp=(char *)malloc(sizeof(char)*dimReq);
        if (exp!=NULL)
        {
            strncpy(exp,buff,dimReq);
            exp[dimReq-1]='\0';
            return exp;
        }
        else printf("Memory allocation error.\n");
    }
    
    return NULL;
}

