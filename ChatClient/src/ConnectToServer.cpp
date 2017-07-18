#include "ConnectToServer.h"

string groupOpt;
string groupName;
string request;
string message;
string filePath;
string clientAlias;
int done;

int ConnectToServer::status = DISCONNECTED;

ConnectToServer& ConnectToServer::getInstance()
{
    static ConnectToServer instance;
    return instance;
}

int  ConnectToServer::getStatus()
{
    return ConnectToServer::status;
}

ConnectToServer::ConnectToServer()
{
    alias = "";
    connector = NULL;
    stream = NULL;
}

string ConnectToServer::getAlias()
{
    return this->alias;
}

void* threadMain(void*)
{
    pthread_detach(pthread_self()); 

    ConnectToServer& client = ConnectToServer::getInstance();
    client.receiveMessage();

    return NULL;
}

void* threadFileTransfer(void* fileStream)
{
    pthread_detach(pthread_self()); 

    ConnectToServer& client = ConnectToServer::getInstance();
    client.transferFile((TCPStream*) fileStream, filePath);

    delete (TCPStream*) fileStream;
    return NULL;
}

int startWith(string str, string prefix)
{
   if (!str.compare(0, prefix.size(), prefix))
   {
        return 1;
   }
   else
   {
        return 0;
   }
}

int checkDirectory(const char* pathName)
{
    struct stat info;

    if(stat( pathName, &info ) != 0)
    {
        printf( "cannot access %s\n", pathName );
        return -1;
    }        
    else if (S_ISDIR(info.st_mode))
    {
        printf( "%s is a directory\n", pathName );
        return -1;
    }        
    else if (S_ISREG(info.st_mode))
    {
        return 0;
    }
    else
    {
        printf( "%s is NOT a regular file\n", pathName );
        return -1;
    }
}

string getFileName(string path)
{
    int pos;
    int temp = 0;
    while (temp >= 0)
    {
        pos = temp;
        temp = path.find("/", pos + 1);
    }

    return path.substr(pos + 1);
}

long getFileSize(const char* fileName)
{
    ifstream in(fileName, ifstream::ate | ifstream::binary);
    return (long)in.tellg(); 
}

int ConnectToServer::transferFile(TCPStream* fileStream, string path)
{
    string fileName = getFileName(path);
    long fileSize = getFileSize(path.c_str());

    int seqNum = 0;
    char buffer[BUFFER_SIZE];
    int rcvMsgSize;
    string rcvMsg;

    string sendMsg = fileName;
    fileStream->send(sendMsg.c_str(), sendMsg.length());
    
    if ((rcvMsgSize = fileStream->receive(buffer, BUFFER_SIZE)) > 0)
    {
        buffer[rcvMsgSize] = '\0';
        rcvMsg = buffer;
        if (stoi(rcvMsg) == seqNum)
        {
            seqNum++;
        }
        else
        {
            cout << "Sending error" << endl;
            return -1;
        }
    }

    sendMsg = to_string(fileSize);
    fileStream->send(sendMsg.c_str(), sendMsg.length());

    if ((rcvMsgSize = fileStream->receive(buffer, BUFFER_SIZE)) > 0)
    {
        buffer[rcvMsgSize] = '\0';
        rcvMsg = buffer;
        if (stoi(rcvMsg) == seqNum)
        {
            seqNum++;
        }
        else
        {
            cout << "Sending file error" << endl;
            return -1;
        }
    }

    FILE *fs = fopen(path.c_str(), "r");
    if(fs == NULL)
    {
        printf("ERROR: File %s not found.\n", path.c_str());
        return -1;
    }

    char sendingBuf[BUFFER_SIZE];
    bzero(sendingBuf, BUFFER_SIZE); 
    int blockSize;

    cout << "Sending file..." << endl;

    while((blockSize = fread(sendingBuf, sizeof(char), BUFFER_SIZE, fs)) > 0)
    {
        cout << blockSize << endl;
        fileStream->send(sendingBuf, blockSize);
        bzero(sendingBuf, BUFFER_SIZE);

        if ((rcvMsgSize = fileStream->receive(buffer, BUFFER_SIZE)) > 0)
        {
            buffer[rcvMsgSize] = '\0';
            rcvMsg = buffer;
            if (stoi(rcvMsg) == seqNum)
            {
                seqNum++;
            }
            else
            {
                cout << "Sending error" << endl;
                break;
            }
        }
        
    }
    fclose(fs);

    printf("File %s  was sent!\n", fileName.c_str());
}

int ConnectToServer::receiveMessage()
{
    if (stream == NULL)
    {
        return -1;
    }

    char buffer[BUFFER_SIZE];
    int rcvMsgSize;
    string rcvMsg;
    string sendMsg;


    while ((rcvMsgSize = stream->receive(buffer, BUFFER_SIZE)) > 0)
    {
        buffer[rcvMsgSize] = '\0';
        rcvMsg = buffer;

        if (rcvMsg == "OK")
        {
            cout << "Connected successfully to Server" << endl;
            cout << "Enter user: ";
            getline(cin, sendMsg);
            stream->send(sendMsg.c_str(), sendMsg.length());
        }
        else if (startWith(rcvMsg, "NEW CLIENT"))
        {
            cout << "Enter alias: ";
            getline(cin, sendMsg);
            stream->send(sendMsg.c_str(), sendMsg.length());            
            alias = sendMsg;
            status = CONNECTED;
        }
        else if (rcvMsg == "GROUP OPT")
        {
            sendMsg = groupOpt;
            stream->send(sendMsg.c_str(), sendMsg.length());
        }
        else if (rcvMsg == "WHICH GROUP")
        {
            sendMsg = groupName;
            stream->send(sendMsg.c_str(), sendMsg.length());
        }
        else if (rcvMsg == "WHICH CLIENT")
        {
            sendMsg = clientAlias;
            stream->send(sendMsg.c_str(), sendMsg.length());
        }
        else if (rcvMsg == "CLIENT REQUEST")
        {
            sendMsg = request;
            stream->send(sendMsg.c_str(), sendMsg.length());
        }
        else if (rcvMsg == "GROUP REQUEST")
        {
            sendMsg = request;
            stream->send(sendMsg.c_str(), sendMsg.length());
        }
        else if (rcvMsg == "CLIENT MESSAGE")
        {
            sendMsg = message;
            stream->send(sendMsg.c_str(), sendMsg.length());
        }
        else if (rcvMsg == "GROUP MESSAGE")
        {
            sendMsg = message;
            stream->send(sendMsg.c_str(), sendMsg.length());
        }
        else if (rcvMsg == "CLIENT FILE")
        {
            TCPConnector* fileConnector = new TCPConnector();
            TCPStream* fileStream = fileConnector->connect(serverIP.c_str(), FILE_PORT);

            if (!fileStream)
            {
                cerr << "Could not connect to Server to send file" << endl;
            }

            pthread_t threadID;
            if (pthread_create(&threadID, NULL, threadFileTransfer, (void *) fileStream) != 0)
            {
                cerr << "Could not create thread" << endl;
            }
            delete fileConnector;
        }
        else if (rcvMsg == "GROUP FILE")
        {
            sendMsg = message;
            stream->send(sendMsg.c_str(), sendMsg.length());
        }
        else if (startWith(rcvMsg, "ALIAS"))
        {
            vector<string> v;
            if (split(rcvMsg, v) != 0)
            {
                cout << "Invalid received alias" << endl;
                return -1;
            }
            alias = v.at(1);
            status = CONNECTED;
        }
        else if (startWith(rcvMsg, "[OK]"))
        {
            if (rcvMsg.length() > 4)
            {
                cout << rcvMsg.substr(4) << endl;
            }

            done = 1;
        }
        else
        {
            cout << rcvMsg << endl;
        }
    }    
    status = DISCONNECTED;

    return 0;
}

int ConnectToServer::connect(string serverIP)
{
    this->serverIP = serverIP;
    connector = new TCPConnector();
    stream = connector->connect(serverIP.c_str(), SERVER_PORT);

    if (!stream)
    {
        return -1;
    }

    pthread_t threadID;
    if (pthread_create(&threadID, NULL, threadMain, NULL) != 0)
    {
        cerr << "Could not create thread" << endl;
        return -1;
    }

    string sendMsg = HEADER_CONNECT;
    stream->send(sendMsg.c_str(), sendMsg.length());

    return 0;
}

int ConnectToServer::groupOperation(string group, string opt)
{
    done = 0;
    if (stream == NULL)
    {
        return -1;
    }

    if (group == "")
    {
        return -1;
    }

    groupName = group;
    groupOpt = opt;

    string sendMsg = HEADER_GROUP_MOD;
    char buffer[BUFFER_SIZE];
    int rcvMsgSize;
    string rcvMsg;

    stream->send(sendMsg.c_str(), sendMsg.length());

    while(!done);

    return 0;
}

int ConnectToServer::split(const string& s, vector<string>& v)
{
    int pos = s.find("::");
    if (pos < 0)
    {
        return -1;
    }
    string alias = s.substr(0, pos);
    string msg = s.substr(pos + 2);

    v.push_back(alias);
    v.push_back(msg);

    return 0;
}
	
int ConnectToServer::headerCompare(string& cmd)
{
    vector<string> lstHeader = { "schat ", "gchat ", "sfile ", "gfile ", "cgroup ", "jgroup ", "lgroup " , "exit" };
    int size = lstHeader.size();
    for (int i = 0; i < size; i++)
    {
        string header = lstHeader.at(i);
        if (cmd.compare(0, header.length(), header) == 0)
        {
            if (header.length() <= cmd.length())
            {
                cmd = cmd.substr(header.length());
            }
            else
            {
                cmd = "";
            }
            return i;
        }
    }
    
    return -1;
}

int ConnectToServer::createGroup(string group)
{
    return groupOperation(group, "Create Group");
}

int ConnectToServer::joinGroup(string group)
{
    return groupOperation(group, "Join in Group");
}

int ConnectToServer::leaveGroup(string group)
{
    return groupOperation(group, "Leave Group");
}

int ConnectToServer::singleChat(string cmd)
{
    done = 0;
    vector<string> v;
    if (split(cmd, v) != 0)
    {
        return -1;
    }

    clientAlias = v.at(0);
    message = v.at(1);
    request = "Send message";

    char buffer[BUFFER_SIZE];
    int rcvMsgSize;
    string sendMsg;
    string rcvMsg;

    sendMsg = HEADER_SINGLE_REQ;
    stream->send(sendMsg.c_str(), sendMsg.length());
    while(!done);

    return 0;
}

int ConnectToServer::groupChat(string cmd)
{
    done = 0;

    vector<string> v;
    if (split(cmd, v) != 0)
    {
        return -1;
    }

    groupName = v.at(0);
    message = v.at(1);
    request = "Send message";

    char buffer[BUFFER_SIZE];
    int rcvMsgSize;
    string sendMsg;
    string rcvMsg;

    sendMsg = HEADER_GROUP_REQ;
    stream->send(sendMsg.c_str(), sendMsg.length());
    while(!done);

    return 0;
}

int ConnectToServer::singleFileTransfer(string cmd)
{
    vector<string> v;
    if (split(cmd, v) != 0)
    {
        return -1;
    }

    if (checkDirectory(v.at(1).c_str()))
    {
        return -1;
    }

    clientAlias = v.at(0);
    filePath = v.at(1);
    request = "Send file";    

    char buffer[BUFFER_SIZE];
    int rcvMsgSize;
    string sendMsg;
    string rcvMsg;

    sendMsg = HEADER_SINGLE_REQ;
    stream->send(sendMsg.c_str(), sendMsg.length());

    return 0;
}

int ConnectToServer::groupFileTransfer(string cmd)
{
    done = 0;
    vector<string> v;
    if (split(cmd, v) != 0)
    {
        return -1;
    }

    groupName = v.at(0);
    filePath = v.at(1);
    request = "Send file";

    if (checkDirectory(filePath.c_str()))
    {
        return -1;
    }

    char buffer[BUFFER_SIZE];
    int rcvMsgSize;
    string sendMsg;
    string rcvMsg;

    sendMsg = HEADER_GROUP_REQ;
    stream->send(sendMsg.c_str(), sendMsg.length());
    while(!done);

    return 0;
}

void ConnectToServer::disconnect()
{
    if (stream != NULL)
    {
        delete stream;
    }
}