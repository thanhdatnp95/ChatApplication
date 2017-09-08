#ifndef _BUBBLESORT_H_
#define _BUBBLESORT_H_

#include "Sort.h"

template <typename T>
class BubbleSort : public Sort<T>
{
public:
    void DoSort(vector<T>&);
};

template <typename T>
void BubbleSort<T>::DoSort(vector<T>& array)
{
    int size = array.size();
    bool swapped;

    for (int i = 0; i < size - 1; i++)
    {
      swapped = false;
      for (int j = 0; j < size - i - 1; j++)
      {
         if (array[j] > array[j + 1])
         {
            this->Swap(array, j, j + 1);
            swapped = true;
         }
      }  
      // If there is not any element swapped, then break
      if (swapped == false)
         break;
    }

    cout << "Array sorted by Bubble Sort:";
    for (int i = 0; i < size; i++)
    {
        cout << " " << array[i];
    }
    cout << endl;
}
#endif //_BUBBLESORT_H_