#include "Group.h"

Group::Group(int id, string name)
{
    this->id = id;
    this->name =  name;
}

Group::~Group()
{
    lstClient.clear();
}

string Group::getName()
{
    return name;
}

void Group::setName(string name)
{
    this->name = name;
}

int Group::getLstStream(vector<TCPStream*>& lstStream)
{
    TCPConnector* connector;
    TCPStream* rcvStream;
    int size = lstClient.size();
     
    for (int i = 0; i < size; i++)
    {
        Client* remoteClient = lstClient.at(i);
        if (remoteClient->getStatus() == ONLINE)
        {
            connector = new TCPConnector();
            rcvStream = connector->connect(remoteClient->getIPAddr().c_str(), remoteClient->getFilePort());
            if (rcvStream != NULL)
            {
                lstStream.push_back(rcvStream);
            }
        }
    }
    return lstStream.size();
}

int Group::getNumOfMem()
{
    return lstClient.size();
}

void Group::addMem(Client* client)
{
    lstClient.push_back(client);
}

void Group::removeMem(int id)
{
    int size = lstClient.size();
    int i;
    for (i = 0; i < size; i++)
    {
        if (lstClient.at(i)->getID() == id)
        {
            lstClient.erase(lstClient.begin() + i);
            return;
        }
    }
}

int Group::checkExistingMem(int clientID)
{
    int size = lstClient.size();
    for (int i = 0; i < size; i++)
    {
        if (lstClient.at(i)->getID() == clientID)
            return 1;
    }

    return 0;
}

void Group::broadcastMessage(Client* client, string msg)
{
    int size = lstClient.size();
    string sendMsg = getName() + "::" + client->getAlias() + ": " + msg;

    for (int i = 0; i < size; i++)
    {
        Client* remoteClient = lstClient.at(i);
        remoteClient->sendMessage(sendMsg);
    }
}