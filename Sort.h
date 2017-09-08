#ifndef _SORT_H_
#define _SORT_H_

#include <iostream>
#include <vector>
using namespace std;

template <typename T>
class Sort 
{
protected:
    void Swap(vector<T>&, int, int);
public:
    virtual void DoSort(vector<T>&) = 0;
};

template <typename T>
void Sort<T>::Swap(vector<T>& array, int x, int y)
{
    int temp = array.at(x);
    array.at(x) = array.at(y);
    array.at(y) = temp;
}
#endif //_SORT_H_