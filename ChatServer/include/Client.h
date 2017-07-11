#ifndef CLIENT_H_
#define CLIENT_H_

#define    ONLINE                          1
#define    OFFLINE                         0
#define    KEEP_ALIVE_INTV                30 //Keep alive interval (s)
#define    BUFFER_SIZE                  1024

#include <string>
#include <ctime>

using namespace std;

class Client
{
    int id;
    string alias;
    string IPAddr;
    string MACAddr;
    int status;

public:
    Client(int, string, string, string);

    int getID();
    string getAlias();
    void setAlias(string);
    string getIPAddr();
    void setIPAddr(string);
    string getMACAddr();
    int getStatus();
    void setStatus(int);
};

#endif /* CLIENT_H_ */