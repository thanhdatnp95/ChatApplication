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

    void addMem(Client*)
    void removeMem(int);
}

#endif /* GROUP_H_ */