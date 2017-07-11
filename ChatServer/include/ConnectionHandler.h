#ifndef CONNECTIONHANDLER_H_
#define CONNECTIONHANDLER_H_

#define        LISTENING_PORT               8888
#define        HEADER_CONNECT          "Connect"
#define        HEADER_KEEP_ALIVE     "KeepAlive"       
#define        HEADER_GROUP_REQ          "Group"

#include <iostream>
#include "Client.h"
#include "Group.h"
#include "TCPStream.h"
#include "TCPAcceptor.h"
#include "pthread.h"

class ConnectionHandler 
{
    int clientID;
    int groupID;
    vector<Client*> lstClient;
    vector<Group*> lstGroup;
    TCPAcceptor* acceptor;

    void *threadMain(void *);
    void handleClient(TCPStream*);
    Client* updateClient(TCPStream*);
    int updateGroup(Client*, TCPStream*);

public:
    ConnectionHandler();
    ~ConnectionHandler();

    void start();
};

#endif /* CONNECTIONHANDLER_H_ */