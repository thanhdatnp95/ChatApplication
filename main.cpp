#include "BubbleSort.h"
#include "QuickSort.h"

int main()
{
    Sort<char>* s1 = new QuickSort<char>();
    Sort<char>* s2 = new BubbleSort<char>();

    char myints1[] = {'a', 't', 'b', 'i', 'f', 'c'};
    vector<char> v1 (myints1, myints1 + sizeof(myints1) / sizeof(char) );

    char myints2[] = {'z', 'g', 'a', 's', 'b'};
    vector<char> v2 (myints2, myints2 + sizeof(myints2) / sizeof(char) );

    s1->DoSort(v1);
    s2->DoSort(v2);
}