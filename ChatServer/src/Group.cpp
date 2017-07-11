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
