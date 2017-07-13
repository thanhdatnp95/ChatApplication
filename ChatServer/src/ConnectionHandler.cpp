#include "ConnectionHandler.h"

ConnectionHandler& ConnectionHandler::getInstance()
{
    static ConnectionHandler instance;
    return instance;
}

ConnectionHandler::ConnectionHandler()
{
    acceptor = new TCPAcceptor(LISTENING_PORT);
    clientID = 0;
    groupID = 0;
}

ConnectionHandler::~ConnectionHandler()
{
    int size = lstClient.size();
    int i;
    for (i = 0; i < size; i++)
    {
        delete lstClient.at(i);
    }
    lstClient.clear();

    size = lstGroup.size();
    for (i = 0; i < size; i++)
    {
        delete lstGroup.at(i);
    }
    lstGroup.clear();

    delete acceptor;
}

void* threadMain(void *stream)
{
    pthread_detach(pthread_self()); 

    ConnectionHandler& handler = ConnectionHandler::getInstance();
    handler.handleClient((TCPStream*) stream);

    delete (TCPStream*) stream;
    return NULL;
}

void ConnectionHandler::start()
{
    cout << "Start listening..." << endl;
    if (acceptor->start() == 0)
    {
        while (true)
        {
            TCPStream *stream = acceptor->accept();
            if (stream != NULL)
            {
                pthread_t threadID;
                if (pthread_create(&threadID, NULL, threadMain, (void *) stream) != 0)
                {
                    cerr << "Could not create thread" << endl;
                }
            }
        }
    }

    cerr << "Could not start the Server" << endl;
    exit(-1);
}

void ConnectionHandler::handleClient(TCPStream* stream)
{
    char buffer[BUFFER_SIZE];
    int rcvMsgSize;
    string rcvMsg;
    Client* client = NULL;

    while ((rcvMsgSize = stream->receive(buffer, BUFFER_SIZE)) > 0)
    {
        buffer[rcvMsgSize] = '\0';
        rcvMsg = buffer;

        if (rcvMsg == HEADER_CONNECT)
        {               
            if ((client = updateClient(stream)) == NULL)
                break;
        }
        else if (rcvMsg == HEADER_KEEP_ALIVE)
        {
            //Reset timer
        }
        else if (rcvMsg == HEADER_GROUP_REQ)
        {
            if (updateGroup(client) != 0)
                break;
        }
        else if (rcvMsg == HEADER_SINGLE_CHAT)
        {
            if (requestSingle(client) != 0)
                break;
        }
        else if (rcvMsg == HEADER_GROUP_CHAT)
        {
            if (requestGroup(client) != 0)
                break;
        }
        else
        {
            cerr << "Received: " << rcvMsg << endl;
        }
    }

    if (client != NULL)
    {
        cout << "[OFFLINE]: " << client->getMACAddr() << "(" << client->getAlias() << ")" << endl;
        client->setStatus(OFFLINE);
        client->setStream(NULL);
    }
}

Client* ConnectionHandler::updateClient(TCPStream* stream)
{
    string msg = "OK";
    stream->send(msg.c_str(), msg.length());

    char buffer[BUFFER_SIZE];
    int rcvMsgSize;

    if ((rcvMsgSize = stream->receive(buffer, BUFFER_SIZE)) == 0)
        return NULL;
    buffer[rcvMsgSize] = '\0';
    string MACAddr = buffer;
    string alias;

    int size = lstClient.size();
    for (int i = 0; i < size; i++)
    {
        if (lstClient.at(i)->getMACAddr() == MACAddr)
        {
            Client* client = lstClient.at(i);
            alias = client->getAlias();
            cout << "[ONLINE]: " << MACAddr << " (" << alias << ")" << endl;

            stream->send(alias.c_str(), alias.length());
            client->setIPAddr(stream->getPeerIP());
            client->setStatus(ONLINE);
            client->setStream(stream);
            return client;
        }
    }

    msg = "NEW CLIENT";
    stream->send(msg.c_str(), msg.length());

    if ((rcvMsgSize = stream->receive(buffer, BUFFER_SIZE)) == 0)
        return NULL;    
    buffer[rcvMsgSize] = '\0';
    alias = buffer;
    cout << "[ONLINE]: " << MACAddr << "(" << alias << ")" << endl;

    Client* client = new Client(clientID++, stream, alias, MACAddr);
    lstClient.push_back(client);

    return client;
}

int ConnectionHandler::updateGroup(Client* client)
{
    if (client == NULL)
    {
        return -1;
    }

    TCPStream* stream = client->getStream();
    string sendMsg = "What do you want";
    char buffer[BUFFER_SIZE];
    int rcvMsgSize;
    string rcvMsg;

    stream->send(sendMsg.c_str(), sendMsg.length());

    if ((rcvMsgSize = stream->receive(buffer, BUFFER_SIZE)) == 0)
        return -1;
    buffer[rcvMsgSize]= '\0';
    rcvMsg = buffer;

    if (rcvMsg == "Create Group")
    {
        sendMsg = "Which Group";
        stream->send(sendMsg.c_str(), sendMsg.length());

        if ((rcvMsgSize = stream->receive(buffer, BUFFER_SIZE)) == 0)
            return -1;
        buffer[rcvMsgSize]= '\0';
        rcvMsg = buffer; 

        Group* group = checkExistingGroup(rcvMsg);
        if (group == NULL)
        {
            group = new Group(groupID++, rcvMsg);
            group->addMem(client);
            lstGroup.push_back(group);

            sendMsg = "Create group successfully";
            stream->send(sendMsg.c_str(), sendMsg.length());
            cout << "[GROUP]: Created group: " << rcvMsg << " (" << client->getAlias() << ")" << endl;
        }
        else
        {
            sendMsg = "Group is existing";
            stream->send(sendMsg.c_str(), sendMsg.length());
        }
    }
    else if (rcvMsg == "Join in Group")
    {
        sendMsg = "Which Group";
        stream->send(sendMsg.c_str(), sendMsg.length());

        if ((rcvMsgSize = stream->receive(buffer, BUFFER_SIZE)) == 0)
            return -1;
        buffer[rcvMsgSize]= '\0';
        rcvMsg = buffer;

        Group* group = checkExistingGroup(rcvMsg);
        if (group == NULL)
        {
            sendMsg = "Group is not existing";
            stream->send(sendMsg.c_str(), sendMsg.length());
        }
        else
        {
            if (group->checkExistingMem(client->getID()))
            {
                sendMsg = "You are already in group";
                stream->send(sendMsg.c_str(), sendMsg.length());
            }
            else
            {
                group->addMem(client);
                sendMsg = "Join in group successfully";
                stream->send(sendMsg.c_str(), sendMsg.length());
                cout << "[GROUP]: Added a member into group: " << rcvMsg << " (" << client->getAlias() << ")" << endl;
            }
        }
    }
    else if (rcvMsg == "Leave Group")
    {
        sendMsg = "Which Group";
        stream->send(sendMsg.c_str(), sendMsg.length());

        if ((rcvMsgSize = stream->receive(buffer, BUFFER_SIZE)) == 0)
            return -1;
        buffer[rcvMsgSize]= '\0';
        rcvMsg = buffer;

        Group* group = checkExistingGroup(rcvMsg);
        if (group == NULL)
        {
            sendMsg = "Group is not existing";
            stream->send(sendMsg.c_str(), sendMsg.length());
        }
        else
        {
            if (group->checkExistingMem(client->getID()))
            {
                group->removeMem(client->getID());
                sendMsg = "Leave group successfully";
                stream->send(sendMsg.c_str(), sendMsg.length());
                cout << "[GROUP]: Removed a member from group: " << rcvMsg << " (" << client->getAlias() << ")" << endl;
            }
            else
            {
                sendMsg = "You are not in this group";
                stream->send(sendMsg.c_str(), sendMsg.length());
            }
        }
    }
    return 0;
}

Group* ConnectionHandler::checkExistingGroup(string groupName)
{
    int size = lstGroup.size();
    for (int i = 0; i < size; i++)
    {
        if (lstGroup.at(i)->getName() == groupName)
            return lstGroup.at(i);
    }
    return NULL;
}

Client* ConnectionHandler::checkExistingClient(string alias)
{
    int size = lstClient.size();
    for (int i = 0; i < size; i++)
    {
        if (lstClient.at(i)->getAlias() == alias)
            return lstClient.at(i);
    }
    return NULL;
}

int ConnectionHandler::requestSingle(Client* client)
{
    if (client == NULL)
    {
        return -1;
    }

    TCPStream* stream = client->getStream();
    string sendMsg = "Which client";
    char buffer[BUFFER_SIZE];
    int rcvMsgSize;
    string rcvMsg;

    stream->send(sendMsg.c_str(), sendMsg.length());

    if ((rcvMsgSize = stream->receive(buffer, BUFFER_SIZE)) == 0)
        return -1;
    buffer[rcvMsgSize]= '\0';
    rcvMsg = buffer;  
    
    Client* remoteClient = checkExistingClient(rcvMsg);
    if (remoteClient != NULL)
    {
        if (remoteClient->getStatus() == ONLINE)
        {
            sendMsg = remoteClient->getIPAddr();
            stream->send(sendMsg.c_str(), sendMsg.length());
        }
        else
        {
            sendMsg = "Offline";
            stream->send(sendMsg.c_str(), sendMsg.length());
        }
    }
    else
    {
        sendMsg = "Invalid alias";
        stream->send(sendMsg.c_str(), sendMsg.length());
    }

    return 0;
}

int ConnectionHandler::requestGroup(Client* client)
{
    if (client == NULL)
    {
        return -1;
    }

    TCPStream* stream = client->getStream();
    string sendMsg = "Which group";
    char buffer[BUFFER_SIZE];
    int rcvMsgSize;
    string rcvMsg;

    stream->send(sendMsg.c_str(), sendMsg.length());

    if ((rcvMsgSize = stream->receive(buffer, BUFFER_SIZE)) == 0)
        return -1;
    buffer[rcvMsgSize]= '\0';
    rcvMsg = buffer;   
    
    Group* group = checkExistingGroup(rcvMsg);
    if (group == NULL)
    {       
        sendMsg = "Invalid group";
        stream->send(sendMsg.c_str(), sendMsg.length());
    }
    else
    {
        if (group->checkExistingMem(client->getID()))
        {            
            sendMsg = "Which request";
            stream->send(sendMsg.c_str(), sendMsg.length());

            if ((rcvMsgSize = stream->receive(buffer, BUFFER_SIZE)) == 0)
                return -1;
            buffer[rcvMsgSize]= '\0';
            rcvMsg = buffer;

            if (rcvMsg == "Send message")
            {
                sendMsg = "Which message";
                stream->send(sendMsg.c_str(), sendMsg.length());

                if ((rcvMsgSize = stream->receive(buffer, BUFFER_SIZE)) == 0)
                    return -1;
                buffer[rcvMsgSize]= '\0';
                rcvMsg = buffer;
                
                group->broadcastMessage(client, rcvMsg);
            }
            else if (rcvMsg == "Send file")
            {
                //Not implemented yet
            }
            else
            {
                return -1;
            }
        }
        else
        {
            sendMsg = "You are not in this group";
            stream->send(sendMsg.c_str(), sendMsg.length());
        }
    }
    return 0;
}
