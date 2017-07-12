#ifndef CLIENT_H_
#define CLIENT_H_

#define    ONLINE                          1
#define    OFFLINE                         0
#define    KEEP_ALIVE_INTV                30 //Keep alive interval (s)
#define    BUFFER_SIZE                  1024

#include <string>
#include <ctime>
#include "TCPStream.h"

using namespace std;

class Client
{
    int id;
    string alias;
    string IPAddr;
    string MACAddr;
    int status;
    TCPStream* stream;

public:
    Client(int, TCPStream*, string, string);

    int getID();
    string getAlias();
    void setAlias(string);
    string getIPAddr();
    void setIPAddr(string);
    string getMACAddr();
    int getStatus();
    void setStatus(int);
    TCPStream* getStream();
    void setStream(TCPStream*);
};

#endif /* CLIENT_H_ */