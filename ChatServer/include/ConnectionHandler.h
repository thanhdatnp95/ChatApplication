#ifndef CONNECTIONHANDLER_H_
#define CONNECTIONHANDLER_H_

#define        LISTENING_PORT                  8888
#define        HEADER_CONNECT             "Connect"
#define        HEADER_KEEP_ALIVE        "KeepAlive"       
#define        HEADER_GROUP_MOD       "ModifyGroup"
#define        HEADER_SINGLE_REQ    "SingleRequest"       
#define        HEADER_GROUP_REQ      "GroupRequest"

#include <iostream>
#include <cstdlib>
#include <ctime>
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

    ConnectionHandler();

    Client* updateClient(TCPStream*);
    int updateGroup(Client*);
    int requestSingle(Client*);
    int requestGroup(Client*);
    Group* checkExistingGroup(string);
    Client* checkExistingClient(string);

public:
    ConnectionHandler(ConnectionHandler const&) = delete;
    void operator=(ConnectionHandler const&) = delete;

    static ConnectionHandler& getInstance();
    ~ConnectionHandler();

    void start();
    void handleClient(TCPStream*);
};

#endif /* CONNECTIONHANDLER_H_ */