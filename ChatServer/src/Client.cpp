#include "Client.h"

Client::Client(int id, string alias, string ip, string mac)
{
    this->id = id;
    this->alias = alias;
    this->IPAddr = ip;
    this->MACAddr = mac;
    status = ONLINE;
}

int Client::getID()
{
    return id;
}

string Client::getAlias()
{
    return alias;
}

void Client::setAlias(string alias)
{
    this->alias = alias;
}

string Client::getIPAddr()
{
    return IPAddr;
}

void Client::setIPAddr(string IPAdrr)
{
    this->IPAddr = IPAddr;
}

string Client::getMACAddr()
{
    return MACAddr;
}

string Client::getStatus()
{
    return status;
}

void Client::setStatus(int status)
{
    this->status = status;
}

void Client::handleConnection(TCPStream* stream)
{
    char rcvBuffer[BUFFER_SIZE];
    int rcvMsgSize;
    // clock_t start;
    // double duration = 0;

    // start = clock();
    while ((recvMsgSize = stream->recv(echoBuffer, BUFFER_SIZE)) > 0)
    {
        // duration = (clock() - start) / (double) CLOCKS_PER_SEC;
        // if (duration > KEEP_ALIVE_INTV)
        // {
        //     break;
        // }
        stream->send(echoBuffer, recvMsgSize);
    }
    setStatus(OFFLINE);
}

