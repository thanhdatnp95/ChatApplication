#ifndef CONECTTOSERVER_H_
#define CONECTTOSERVER_H_

#define        SERVER_PORT                     8888
#define        FILE_PORT                       9999
#define        BUFFER_SIZE                     1024
#define        CONNECTED                          1
#define        DISCONNECTED                       0
#define        HEADER_CONNECT             "Connect"
#define        HEADER_KEEP_ALIVE        "KeepAlive"       
#define        HEADER_GROUP_MOD       "ModifyGroup"
#define        HEADER_SINGLE_REQ    "SingleRequest"       
#define        HEADER_GROUP_REQ      "GroupRequest"

#include <string>
#include <string.h>
#include <iostream>
#include <vector>
#include <pthread.h>
#include <sys/stat.h>
#include <fstream>
#include "TCPConnector.h"
#include "TCPStream.h"

using namespace std;

class ConnectToServer
{
    string alias;
    string serverIP;
    TCPConnector* connector;
    TCPStream* stream;
    static int status;

    ConnectToServer();

    int groupOperation(string, string);
    int split(const string&, vector<string>&);

public:
    ConnectToServer(ConnectToServer const&) = delete;
    void operator=(ConnectToServer const&) = delete;

    static ConnectToServer& getInstance();
    string getAlias();
    int connect(string);
    int receiveMessage();
    int headerCompare(string&);
    int createGroup(string);
    int joinGroup(string);
    int leaveGroup(string);
    int singleChat(string);
    int groupChat(string);
    int singleFileTransfer(string);
    int groupFileTransfer(string);
    int transferFile(TCPStream*, string);
    void disconnect();
    int getStatus();
};

#endif /* CONECTTOSERVER_H_ */