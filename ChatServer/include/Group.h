#ifndef GROUP_H_
#define GROUP_H_

#include <string>
#include <vector>
#include "Client.h"

using namespace std;

class Group 
{
    int id;
    string name;
    vector<Client*> lstClient;

public:
    Group(int, string);
    ~Group();
    
    string getName();
    void setName(string);
    int getNumOfMem();

    void addMem(Client*);
    void removeMem(int);
    int checkExistingMem(int);
    void broadcastMessage(Client*, string);
};

#endif /* GROUP_H_ */