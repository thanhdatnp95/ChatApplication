#include "ConnectToServer.h"

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

int ConnectToServer::connect(string serverIP)
{
    connector = new TCPConnector();
    stream = connector->connect(serverIP.c_str(), SERVER_PORT);

    if (!stream)
    {
        return -1;
    }    

    char buffer[BUFFER_SIZE];
    int rcvMsgSize;
    string sendMsg;
    string rcvMsg;

    sendMsg = HEADER_CONNECT;
    stream->send(sendMsg.c_str(), sendMsg.length());

    if ((rcvMsgSize = stream->receive(buffer, BUFFER_SIZE)) == 0)
    {
        return -1;
    }
    buffer[rcvMsgSize] = '\0';
    rcvMsg = buffer;

    if (rcvMsg == "OK")
    {
        cout << "Connected successfully to Server" << endl;
        cout << "Enter user: ";
        getline(cin, sendMsg);
        stream->send(sendMsg.c_str(), sendMsg.length());

        if ((rcvMsgSize = stream->receive(buffer, BUFFER_SIZE)) == 0)
        {
            return -1;
        }
        buffer[rcvMsgSize] = '\0';
        rcvMsg = buffer;

        if (rcvMsg == "NEW CLIENT")
        {
            cout << "Enter alias: ";
            getline(cin, sendMsg);
            stream->send(sendMsg.c_str(), sendMsg.length());

            alias = sendMsg;
        }
        else
        {            
            alias = rcvMsg;
        }        

        return 0;        
    }
    else
    {
        return -1;
    }
    
}

int ConnectToServer::groupOpt(string group, string opt)
{
    if (stream == NULL)
    {
        return -1;
    }

    if (group == "")
    {
        return -1;
    }
    string sendMsg = HEADER_GROUP_REQ;
    char buffer[BUFFER_SIZE];
    int rcvMsgSize;
    string rcvMsg;

    stream->send(sendMsg.c_str(), sendMsg.length());
    
    if ((rcvMsgSize = stream->receive(buffer, BUFFER_SIZE)) == 0)
    {
        return -1;
    }
    buffer[rcvMsgSize] = '\0';
    rcvMsg = buffer;

    if (rcvMsg == "What do you want")
    {
        sendMsg = opt;

        stream->send(sendMsg.c_str(), sendMsg.length());
    
        if ((rcvMsgSize = stream->receive(buffer, BUFFER_SIZE)) == 0)
        {
            return -1;
        }
        buffer[rcvMsgSize] = '\0';
        rcvMsg = buffer;

        if (rcvMsg == "Which Group")
        {
            sendMsg = group;

            stream->send(sendMsg.c_str(), sendMsg.length());
        
            if ((rcvMsgSize = stream->receive(buffer, BUFFER_SIZE)) == 0)
            {
                return -1;
            }
            buffer[rcvMsgSize] = '\0';
            rcvMsg = buffer;

            cout << rcvMsg << endl;

            return 0;
        }
        else
        {
            return -1;
        }
    }
    else
    {
        return -1;
    }
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
    vector<string> lstHeader = { "schat ", "gchat ", "cgroup ", "jgroup ", "lgroup " , "exit" };
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
    return groupOpt(group, "Create Group");
}

int ConnectToServer::joinGroup(string group)
{
    return groupOpt(group, "Join in Group");
}

int ConnectToServer::leaveGroup(string group)
{
    return groupOpt(group, "Leave Group");
}

int ConnectToServer::singleChat(string cmd)
{
    vector<string> v;
    if (split(cmd, v) != 0)
    {
        return -1;
    }

    string alias = v.at(0);
    string msg = v.at(1);

    char buffer[BUFFER_SIZE];
    int rcvMsgSize;
    string sendMsg;
    string rcvMsg;

    sendMsg = HEADER_SINGLE_CHAT;
    stream->send(sendMsg.c_str(), sendMsg.length());

    if ((rcvMsgSize = stream->receive(buffer, BUFFER_SIZE)) == 0)
    {
        return -1;
    }
    buffer[rcvMsgSize] = '\0';
    rcvMsg = buffer;  

    if (rcvMsg == "Which client")
    {
        sendMsg = alias;
        stream->send(sendMsg.c_str(), sendMsg.length());

        if ((rcvMsgSize = stream->receive(buffer, BUFFER_SIZE)) == 0)
        {
            return -1;
        }   
        buffer[rcvMsgSize] = '\0';
        rcvMsg = buffer;

        if (rcvMsg == "Invalid alias")
        {
            cout << "Invalid user" << endl;
        }
        else if (rcvMsg == "Offline")
        {
            cout << alias << " is currently offline" << endl;
        }
        else
        {
            //Not implemented yet
            cout << "Sending to " << rcvMsg << endl;
        }

        return 0; 
    } 
    else
    {
        return -1;
    }
}

int ConnectToServer::groupChat(string cmd)
{
    vector<string> v;
    if (split(cmd, v) != 0)
    {
        return -1;
    }

    string group = v.at(0);
    string msg = v.at(1);

    char buffer[BUFFER_SIZE];
    int rcvMsgSize;
    string sendMsg;
    string rcvMsg;

    sendMsg = HEADER_GROUP_CHAT;
    stream->send(sendMsg.c_str(), sendMsg.length());

    if ((rcvMsgSize = stream->receive(buffer, BUFFER_SIZE)) == 0)
    {
        return -1;
    }
    buffer[rcvMsgSize] = '\0';
    rcvMsg = buffer;  

    if (rcvMsg == "Which group")
    {
        sendMsg = group;
        stream->send(sendMsg.c_str(), sendMsg.length());

        if ((rcvMsgSize = stream->receive(buffer, BUFFER_SIZE)) == 0)
        {
            return -1;
        }   
        buffer[rcvMsgSize] = '\0';
        rcvMsg = buffer;

        if (rcvMsg == "Which request")
        {
            sendMsg = "Send message";
            stream->send(sendMsg.c_str(), sendMsg.length());

            if ((rcvMsgSize = stream->receive(buffer, BUFFER_SIZE)) == 0)
            {
                return -1;
            }   
            buffer[rcvMsgSize] = '\0';
            rcvMsg = buffer;

            if (rcvMsg == "Which message")
            {
                stream->send(msg.c_str(), sendMsg.length());
            }
            else
            {
                return -1;
            }
        }
        else
        {
            cout << rcvMsg << endl;
        }

        return 0; 
    } 
    else
    {
        return -1;
    }
}

void ConnectToServer::disconnect()
{
    if (stream != NULL)
    {
        delete stream;
    }
}