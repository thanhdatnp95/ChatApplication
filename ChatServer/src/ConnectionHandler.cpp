#include "ConnectionHandler.h"

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

void ConnectionHandler::start()
{
    if (acceptor->start() == 0)
    {
        while (true)
        {
            TCPStream *stream = acceptor.accept();
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

void* ConnectionHandler::threadMain(void *stream)
{
    pthread_detach(pthread_self()); 
 
    handleClient((TCPStream*) stream);

    delete (TCPSocket*) stream;
    return NULL;
}

void ConnectionHandler::handleClient(TCPStream* stream)
{
    char buffer[BUFFER_SIZE];
    int rcvMsgSize;
    Client* client = NULL;

    while ((recvMsgSize = stream->receive(buffer, BUFFER_SIZE)) > 0)
    { 
        if (buffer == HEADER_CONNECT)
        {        
            if (client = updateClient(stream) == NULL)
                break;
        }
        else if (buffer == HEADER_KEEP_ALIVE)
        {
            //Reset timer
        }
        else if (buffer == HEADER_GROUP_REQ)
        {
            if (updateGroup(client) != 0)
                break;
        }
        else
        {
            cerr << "Received: " << buffer << end;
        }
    }

    if (client != NULL)
    {
        client->setStatus(OFFLINE);
        client->setStream(NULL);
    }
}

Client* ConnectionHandler::updateClient(TCPStream* stream)
{
    string MACAddr;
    if (stream->receive(MACAddr, BUFFER_SIZE) == 0)
        return NULL;
    
    int size = lstClient.size();
    for (int i = 0; i < size; i++)
    {
        if (lstClient.at(i)->getMACAddr() == MACAddr)
        {
            Client* client = lstClient.at(i);
            string alias = client->getAlias();

            stream->send(alias, alias.length());
            client->setIPAddr(stream->getPeerIP());
            client->setStatus(ONLINE);
            return client;
        }
    }

    string msg = "NEW CLIENT";
    stream->send(msg, msg.length());

    string alias;
    if (stream->receive(alias, BUFFER_SIZE) == 0)
        return NULL;
    
    
    Client* client = new Client(clientID++, alias, stream->getPeerIP(), MACAddr);
    lstClient.push_back(client);

    return client;
}

int updateGroup(Client* client)
{
    if (client == NULL)
    {
        return -1;
    }
    string MACAddr;
    if (stream->receive(MACAddr, BUFFER_SIZE) == 0)
        return -1;

    
    return 0;
}