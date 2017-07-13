#ifndef CONECTTOSERVER_H_
#define CONECTTOSERVER_H_

#define        SERVER_PORT                  8888
#define        BUFFER_SIZE                  1024
#define        HEADER_CONNECT          "Connect"
#define        HEADER_KEEP_ALIVE     "KeepAlive"       
#define        HEADER_GROUP_REQ    "ModifyGroup"
#define        HEADER_SINGLE_CHAT   "SingleChat"       
#define        HEADER_GROUP_CHAT     "GroupChat"

#include <string>
#include <iostream>
#include <vector>
#include "TCPConnector.h"
#include "TCPStream.h"

using namespace std;

class ConnectToServer
{
    string alias;
    TCPConnector* connector;
    TCPStream* stream;

    int groupOpt(string, string);
    int split(const string&, vector<string>&);

public:
    ConnectToServer();
    string getAlias();
    int connect(string);
    int headerCompare(string&);
    int createGroup(string);
    int joinGroup(string);
    int leaveGroup(string);
    int singleChat(string);
    int groupChat(string);
    void disconnect();

};

#endif /* CONECTTOSERVER_H_ */