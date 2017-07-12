#include "Client.h"

Client::Client(int id, TCPStream* stream, string alias, string mac)
{
    this->id = id;
    this->stream = stream;
    this->alias = alias;
    this->IPAddr = stream->getPeerIP();
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

int Client::getStatus()
{
    return status;
}

void Client::setStatus(int status)
{
    this->status = status;
}

TCPStream* Client::getStream()
{
    return this->stream;
}

void Client::setStream(TCPStream* stream)
{
    this->stream = stream;
}
