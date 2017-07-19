#include "ConnectionHandler.h"

ConnectionHandler& ConnectionHandler::getInstance()
{
    static ConnectionHandler instance;
    return instance;
}

ConnectionHandler::ConnectionHandler()
{
    acceptor = new TCPAcceptor(LISTENING_PORT);
    fileAcceptor = new TCPAcceptor(SERVER_FILE_PORT);
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

void* threadFileTransfer(void *stream)
{
    pthread_detach(pthread_self()); 

    ConnectionHandler& handler = ConnectionHandler::getInstance();
    handler.handleFileTransfer((TCPStream*) stream);

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
        else if (rcvMsg == HEADER_GROUP_MOD)
        {
            if (updateGroup(client) != 0)
                break;
        }
        else if (rcvMsg == HEADER_SINGLE_REQ)
        {
            if (requestSingle(client) != 0)
                break;
        }
        else if (rcvMsg == HEADER_GROUP_REQ)
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
        time_t currentTime;
        struct tm *localTime;

        time( &currentTime );                   // Get the current time
        localTime = localtime( &currentTime );  // Convert the current time to the local time

        int Day    = localTime->tm_mday;
        int Month  = localTime->tm_mon + 1;
        int Year   = localTime->tm_year + 1900;
        int Hour   = localTime->tm_hour;
        int Min    = localTime->tm_min;
        int Sec    = localTime->tm_sec;

        cout << "[" << Month << "/" << Day << "/" << Year << "-" << Hour << ":" << Min << ":" << Sec << "]"
             << "[OFFLINE]: " << client->getMACAddr() << "(" << client->getAlias() << ")" << endl;
        client->setStatus(OFFLINE);
        client->setStream(NULL);
    }
}

void ConnectionHandler::handleFileTransfer(TCPStream* stream)
{
    string fileName;
    string fileSize;
    string rcvObj;
    string rcvName;
    string sendName;
    long size;

    char buffer[BUFFER_SIZE];
    int rcvMsgSize;
    int rcvSize;
    string rcvMsg;
    int seqNum = 0;
    string sendMsg;

    if ((rcvMsgSize = stream->receive(buffer, BUFFER_SIZE)) > 0)
    {
        buffer[rcvMsgSize] = '\0';
        rcvObj = buffer;
    }

    sendMsg = to_string(seqNum);
    stream->send(sendMsg.c_str(), sendMsg.length());
    seqNum++;

    if ((rcvMsgSize = stream->receive(buffer, BUFFER_SIZE)) > 0)
    {
        buffer[rcvMsgSize] = '\0';
        rcvName = buffer;
    }

    TCPConnector* connector;
    TCPStream* rcvStream;
    if (rcvObj == "Client")
    {
        Client* rcvClient = checkExistingClient(rcvName);
        if (rcvClient != NULL)
        {
            connector = new TCPConnector();
            rcvStream = connector->connect(rcvClient->getIPAddr().c_str(), rcvClient->getFilePort());
        }
    }
    else if (rcvObj == "Group")
    {
        vector<TCPStream*> lstStream;
        int numOfClient;
        Group* rcvGroup = checkExistingGroup(rcvName);
        if (rcvGroup != NULL)
        {            
            if (!(numOfClient = rcvGroup->getLstStream(lstStream)))
            {
                cout << "All members  of group are offline" << endl;
                return;
            }
            cout << "There are " << numOfClient << " member in group" << endl;                        
        }
        cout << "0" << endl;

        sendMsg = to_string(seqNum);
        stream->send(sendMsg.c_str(), sendMsg.length());
        seqNum++;

        if ((rcvMsgSize = stream->receive(buffer, BUFFER_SIZE)) > 0)
        {
            buffer[rcvMsgSize] = '\0';
            sendName = buffer;
            sendName = rcvName + "::" + sendName;

            for (int i = 0; i < numOfClient; i++)
            {
                lstStream.at(i)->send(sendName.c_str(), sendName.length());
                if ((rcvSize = lstStream.at(i)->receive(buffer, BUFFER_SIZE)) > 0)
                {
                    buffer[rcvSize] = '\0';
                    rcvMsg = buffer;
                    if (stoi(rcvMsg) != seqNum)
                    {
                        cout << "Sending error" << endl;    
                    }
                }
            }
        }
        cout << "1" << endl;
        sendMsg = to_string(seqNum);
        stream->send(sendMsg.c_str(), sendMsg.length());
        seqNum++;

        if ((rcvMsgSize = stream->receive(buffer, BUFFER_SIZE)) > 0)
        {
            buffer[rcvMsgSize] = '\0';
            fileName = buffer;
            
            for (int i = 0; i < numOfClient; i++)
            {
                lstStream.at(i)->send(buffer, rcvMsgSize);
                if ((rcvSize = lstStream.at(i)->receive(buffer, BUFFER_SIZE)) > 0)
                {
                    buffer[rcvSize] = '\0';
                    rcvMsg = buffer;
                    if (stoi(rcvMsg) != seqNum)
                    {
                        cout << "Sending error" << endl;    
                    }
                }
            }
        }
        cout << "2" << endl;
        sendMsg = to_string(seqNum);
        stream->send(sendMsg.c_str(), sendMsg.length());
        seqNum++;

        if ((rcvMsgSize = stream->receive(buffer, BUFFER_SIZE)) > 0)
        {
            buffer[rcvMsgSize] = '\0';
            fileSize = buffer;
            size = stol(fileSize);

            cout << "filesize: " << size << endl;
            cout << numOfClient << endl;
            
            for (int i = 0; i < numOfClient; i++)
            {
                lstStream.at(i)->send(buffer, rcvMsgSize);
                if ((rcvSize = lstStream.at(i)->receive(buffer, BUFFER_SIZE)) > 0)
                {
                    buffer[rcvSize] = '\0';
                    rcvMsg = buffer;
                    if (stoi(rcvMsg) != seqNum)
                    {
                        cout << "Sending error" << endl;    
                    }
                }
            }
        }
        cout << "3" << endl;
        sendMsg = to_string(seqNum);
        stream->send(sendMsg.c_str(), sendMsg.length());
        seqNum++;

        char fileBuffer[BUFFER_SIZE];
        int bytesRead;

        cout << "Start receiving file..." << endl;
        while (size)
        {
            bytesRead = stream->receive(fileBuffer, BUFFER_SIZE > size ? size : BUFFER_SIZE);
            if (bytesRead < 0)
            {
                cout << "Receiving file error" << endl;
                break;
            }

            for (int i = 0; i < numOfClient; i++)
            {
                lstStream.at(i)->send(fileBuffer, bytesRead);
                if ((rcvMsgSize = lstStream.at(i)->receive(buffer, BUFFER_SIZE)) > 0)
                {
                    buffer[rcvSize] = '\0';
                    rcvMsg = buffer;
                    if (stoi(rcvMsg) != seqNum)
                    {
                        cout << "Sending error" << endl;    
                    }
                }
            }
            sendMsg = to_string(seqNum);
            stream->send(sendMsg.c_str(), sendMsg.length());
            seqNum++;

            size -= bytesRead;      
        }
        return;
    }

    sendMsg = to_string(seqNum);
    stream->send(sendMsg.c_str(), sendMsg.length());
    seqNum++;

    if ((rcvMsgSize = stream->receive(buffer, BUFFER_SIZE)) > 0)
    {
        buffer[rcvMsgSize] = '\0';
        sendName = buffer;
        rcvStream->send(buffer, rcvMsgSize);
    }

    if ((rcvMsgSize = rcvStream->receive(buffer, BUFFER_SIZE)) > 0)
    {
        buffer[rcvMsgSize] = '\0';
        rcvMsg = buffer;
        if (stoi(rcvMsg) == seqNum)
        {
            sendMsg = to_string(seqNum);
            stream->send(sendMsg.c_str(), sendMsg.length());
            seqNum++;
        }
        else
        {
            cout << "Sending error" << endl;
        }
    }

    if ((rcvMsgSize = stream->receive(buffer, BUFFER_SIZE)) > 0)
    {
        buffer[rcvMsgSize] = '\0';
        fileName = buffer;
        rcvStream->send(buffer, rcvMsgSize);
    }

    if ((rcvMsgSize = rcvStream->receive(buffer, BUFFER_SIZE)) > 0)
    {
        buffer[rcvMsgSize] = '\0';
        rcvMsg = buffer;
        if (stoi(rcvMsg) == seqNum)
        {
            sendMsg = to_string(seqNum);
            stream->send(sendMsg.c_str(), sendMsg.length());
            seqNum++;
        }
        else
        {
            cout << "Sending error" << endl;
        }
    }

    if ((rcvMsgSize = stream->receive(buffer, BUFFER_SIZE)) > 0)
    {
        buffer[rcvMsgSize] = '\0';
        fileSize = buffer;
        rcvStream->send(buffer, rcvMsgSize);
        size = stol(fileSize);
    }

    if ((rcvMsgSize = rcvStream->receive(buffer, BUFFER_SIZE)) > 0)
    {
        buffer[rcvMsgSize] = '\0';
        rcvMsg = buffer;
        if (stoi(rcvMsg) == seqNum)
        {
            sendMsg = to_string(seqNum);
            stream->send(sendMsg.c_str(), sendMsg.length());
            seqNum++;
        }
        else
        {
            cout << "Sending error" << endl;
        }
    }
    
    char fileBuffer[BUFFER_SIZE];
    int bytesRead;

    while (size)
    {
        bytesRead = stream->receive(fileBuffer, BUFFER_SIZE > size ? size : BUFFER_SIZE);
        if (bytesRead < 0)
        {
            cout << "Receiving file error" << endl;
            break;
        }

        rcvStream->send(fileBuffer, bytesRead);

        size -= bytesRead;

        if ((rcvMsgSize = rcvStream->receive(buffer, BUFFER_SIZE)) > 0)
        {
            buffer[rcvMsgSize] = '\0';
            rcvMsg = buffer;
            if (stoi(rcvMsg) == seqNum)
            {
                sendMsg = to_string(seqNum);
                stream->send(sendMsg.c_str(), sendMsg.length());
                seqNum++;
            }
            else
            {
                cout << "Sending error" << endl;
            }
        }        
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

    msg = "FILE_PORT";
    stream->send(msg.c_str(), msg.length());

    if ((rcvMsgSize = stream->receive(buffer, BUFFER_SIZE)) == 0)
        return NULL;
    buffer[rcvMsgSize] = '\0';
    string filePort = buffer;
    

    time_t currentTime;
    struct tm *localTime;

    time( &currentTime );                   // Get the current time
    localTime = localtime( &currentTime );  // Convert the current time to the local time

    int Day    = localTime->tm_mday;
    int Month  = localTime->tm_mon + 1;
    int Year   = localTime->tm_year + 1900;
    int Hour   = localTime->tm_hour;
    int Min    = localTime->tm_min;
    int Sec    = localTime->tm_sec;

    int size = lstClient.size();
    for (int i = 0; i < size; i++)
    {
        if (lstClient.at(i)->getMACAddr() == MACAddr)
        {
            Client* client = lstClient.at(i);
            alias = client->getAlias();
            cout << "[" << Month << "/" << Day << "/" << Year << "-" << Hour << ":" << Min << ":" << Sec << "]"
                 << "[ONLINE]: " << MACAddr << " (" << alias << ")" << endl;

            alias = "ALIAS::" + alias;
            stream->send(alias.c_str(), alias.length());
            client->setIPAddr(stream->getPeerIP());
            client->setStatus(ONLINE);
            client->setStream(stream);
            client->setFilePort(stoi(filePort));
            return client;
        }
    }

    msg = "NEW CLIENT";
    stream->send(msg.c_str(), msg.length());

    if ((rcvMsgSize = stream->receive(buffer, BUFFER_SIZE)) == 0)
        return NULL;    
    buffer[rcvMsgSize] = '\0';
    alias = buffer;
    cout << "[" << Month << "/" << Day << "/" << Year << "-" << Hour << ":" << Min << ":" << Sec << "]"
         << "[ONLINE]: " << MACAddr << "(" << alias << ")" << endl;

    Client* client = new Client(clientID++, stream, alias, MACAddr, stoi(filePort));
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
    string sendMsg = "GROUP OPT";
    char buffer[BUFFER_SIZE];
    int rcvMsgSize;
    string rcvMsg;

    stream->send(sendMsg.c_str(), sendMsg.length());

    if ((rcvMsgSize = stream->receive(buffer, BUFFER_SIZE)) == 0)
        return -1;
    buffer[rcvMsgSize]= '\0';
    rcvMsg = buffer;
    
    time_t currentTime;
    struct tm *localTime;

    time( &currentTime );                   // Get the current time
    localTime = localtime( &currentTime );  // Convert the current time to the local time

    int Day    = localTime->tm_mday;
    int Month  = localTime->tm_mon + 1;
    int Year   = localTime->tm_year + 1900;
    int Hour   = localTime->tm_hour;
    int Min    = localTime->tm_min;
    int Sec    = localTime->tm_sec;

    if (rcvMsg == "Create Group")
    {
        sendMsg = "WHICH GROUP";
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

            sendMsg = "[OK]Create group successfully";
            stream->send(sendMsg.c_str(), sendMsg.length());
            cout << "[" << Month << "/" << Day << "/" << Year << "-" << Hour << ":" << Min << ":" << Sec << "]"
                 << "[GROUP]: Created group: " << rcvMsg << " (" << client->getAlias() << ")" << endl;
        }
        else
        {
            sendMsg = "[OK]Group is existing";
            stream->send(sendMsg.c_str(), sendMsg.length());
        }
    }
    else if (rcvMsg == "Join in Group")
    {
        sendMsg = "WHICH GROUP";
        stream->send(sendMsg.c_str(), sendMsg.length());

        if ((rcvMsgSize = stream->receive(buffer, BUFFER_SIZE)) == 0)
            return -1;
        buffer[rcvMsgSize]= '\0';
        rcvMsg = buffer;

        Group* group = checkExistingGroup(rcvMsg);
        if (group == NULL)
        {
            sendMsg = "[OK]Group is not existing";
            stream->send(sendMsg.c_str(), sendMsg.length());
        }
        else
        {
            if (group->checkExistingMem(client->getID()))
            {
                sendMsg = "[OK]You are already in group";
                stream->send(sendMsg.c_str(), sendMsg.length());
            }
            else
            {
                group->addMem(client);
                sendMsg = "[OK]Join in group successfully";
                stream->send(sendMsg.c_str(), sendMsg.length());
                cout << "[" << Month << "/" << Day << "/" << Year << "-" << Hour << ":" << Min << ":" << Sec << "]"
                     << "[GROUP]: Added a member into group: " << rcvMsg << " (" << client->getAlias() << ")" << endl;
            }
        }
    }
    else if (rcvMsg == "Leave Group")
    {
        sendMsg = "WHICH GROUP";
        stream->send(sendMsg.c_str(), sendMsg.length());

        if ((rcvMsgSize = stream->receive(buffer, BUFFER_SIZE)) == 0)
            return -1;
        buffer[rcvMsgSize]= '\0';
        rcvMsg = buffer;

        Group* group = checkExistingGroup(rcvMsg);
        if (group == NULL)
        {
            sendMsg = "[OK]Group is not existing";
            stream->send(sendMsg.c_str(), sendMsg.length());
        }
        else
        {
            if (group->checkExistingMem(client->getID()))
            {
                group->removeMem(client->getID());
                sendMsg = "[OK]Leave group successfully";
                stream->send(sendMsg.c_str(), sendMsg.length());
                cout << "[" << Month << "/" << Day << "/" << Year << "-" << Hour << ":" << Min << ":" << Sec << "]"
                     << "[GROUP]: Removed a member from group: " << rcvMsg << " (" << client->getAlias() << ")" << endl;
            }
            else
            {
                sendMsg = "[OK]You are not in this group";
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
    string sendMsg = "WHICH CLIENT";
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
            sendMsg = "CLIENT REQUEST";
            stream->send(sendMsg.c_str(), sendMsg.length());

            if ((rcvMsgSize = stream->receive(buffer, BUFFER_SIZE)) == 0)
                return -1;
            buffer[rcvMsgSize]= '\0';
            rcvMsg = buffer;

            if (rcvMsg == "Send message")
            {
                sendMsg = "CLIENT MESSAGE";
                stream->send(sendMsg.c_str(), sendMsg.length());

                if ((rcvMsgSize = stream->receive(buffer, BUFFER_SIZE)) == 0)
                    return -1;
                buffer[rcvMsgSize]= '\0';
                rcvMsg = buffer;
                rcvMsg = client->getAlias() + ": " + rcvMsg;
                sendMsg = "[OK]";
                stream->send(sendMsg.c_str(), sendMsg.length());

                remoteClient->sendMessage(rcvMsg);
            }
            else if (rcvMsg == "Send file")
            {              
                if (fileAcceptor->start() == 0)
                {
                    sendMsg = "CLIENT FILE";
                    stream->send(sendMsg.c_str(), sendMsg.length());

                    TCPStream *fileStream = fileAcceptor->accept();
                    if (fileStream != NULL)
                    {
                        pthread_t threadID;
                        if (pthread_create(&threadID, NULL, threadFileTransfer, (void *) fileStream) != 0)
                        {
                            cerr << "Could not create thread" << endl;
                        }
                    }
                }                
            }
            else
            {
                return -1;
            }
        }
        else
        {
            sendMsg = "[OK]" + rcvMsg + " is currently offline";
            stream->send(sendMsg.c_str(), sendMsg.length());
        }
    }
    else
    {
        sendMsg = "[OK]Invalid alias";
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
    string sendMsg = "WHICH GROUP";
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
        sendMsg = "[OK]Invalid group";
        stream->send(sendMsg.c_str(), sendMsg.length());
    }
    else
    {
        if (group->checkExistingMem(client->getID()))
        {            
            sendMsg = "GROUP REQUEST";
            stream->send(sendMsg.c_str(), sendMsg.length());

            if ((rcvMsgSize = stream->receive(buffer, BUFFER_SIZE)) == 0)
                return -1;
            buffer[rcvMsgSize]= '\0';
            rcvMsg = buffer;

            if (rcvMsg == "Send message")
            {
                sendMsg = "GROUP MESSAGE";
                stream->send(sendMsg.c_str(), sendMsg.length());

                if ((rcvMsgSize = stream->receive(buffer, BUFFER_SIZE)) == 0)
                    return -1;
                buffer[rcvMsgSize]= '\0';
                rcvMsg = buffer;
                sendMsg = "[OK]";
                stream->send(sendMsg.c_str(), sendMsg.length());
                
                group->broadcastMessage(client, rcvMsg);
            }
            else if (rcvMsg == "Send file")
            {
                if (fileAcceptor->start() == 0)
                {
                    sendMsg = "GROUP FILE";
                    stream->send(sendMsg.c_str(), sendMsg.length());

                    TCPStream *fileStream = fileAcceptor->accept();
                    if (fileStream != NULL)
                    {
                        pthread_t threadID;
                        if (pthread_create(&threadID, NULL, threadFileTransfer, (void *) fileStream) != 0)
                        {
                            cerr << "Could not create thread" << endl;
                        }
                    }
                }
            }
            else
            {
                return -1;
            }
        }
        else
        {
            sendMsg = "[OK]You are not in this group";
            stream->send(sendMsg.c_str(), sendMsg.length());
        }
    }
    return 0;
}
