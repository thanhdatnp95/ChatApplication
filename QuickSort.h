#ifndef _QUICKSORT_H_
#define _QUICKSORT_H_

#include "Sort.h"

template <typename T>
class QuickSort : public Sort<T>
{
    int Partition (vector<T>&, int, int);
public:
    void DoSort(vector<T>&);
};

template <typename T>
int QuickSort<T>::Partition (vector<T>& array, int low, int high)
{
    int pivot = array[high];
    int i = (low - 1);
 
    for (int j = low; j <= high - 1; j++)
    {
        if (array[j] <= pivot)
        {
            i++;
            this->Swap(array, i, j);
        }
    }
    this->Swap(array, i + 1, high);

    return (i + 1);
}

template <typename T>
void QuickSort<T>::DoSort(vector<T>& array)
{
    int l = 0;
    int size = array.size();
    int h = size - 1;

    int stack[ h - l + 1 ];
    int top = -1;
    
    stack[++top] = l;
    stack[++top] = h;
    
    // Keep popping from stack while is not empty
    while (top >= 0)
    {
        // Pop h and l
        h = stack[top--];
        l = stack[top--];
    
        // Set pivot element at its correct position
        // in sorted array
        int pivot = Partition(array, l, h);
    
        // If there are elements on left side of pivot,
        // then push left side to stack
        if (pivot - 1 > l)
        {
            stack[++top] = l;
            stack[++top] = pivot - 1;
        }
    
        // If there are elements on right side of pivot,
        // then push right side to stack
        if (pivot + 1 < h)
        {
            stack[++top]  = pivot + 1;
            stack[++top] = h;
        }
    }

    cout << "Array sorted by Quick Sort:";
    for (int i = 0; i < size; i++)
    {
        cout << " " << array[i];
    }
    cout << endl;
}
#endif //_QUICKSORT_H_
