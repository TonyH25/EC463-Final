// Tony Harrington, Jadiel Arana
// This program applies a Linear Classification to hand-drawn numbers

#include <stdio.h>
#include "weights_fp.h"
#include "Image.h"
#include "address_map_arm.h"
typedef char (*img_type)[28];
typedef float (*weight)[28];

int max(int*, int); //parameters are array of ints and length of array

int main(unsigned char img [28][28])
{
  //Create output source
  volatile int* SW_ptr = SW_BASE;
  volatile int* KEY_ptr = KEY_BASE;
  volatile int* LED_ptr = LEDR_BASE;
  //Create array of 10 potential digits
  int output[10];
  img_type img[10] = {Image1, Image2, Image3, Image4, Image5, Image6, Image7, Image8, Image9, Image10};
  weight weights[10] = {weights0, weights1, weights2, weights3, weights4, weights5, weights6, weights7, weights8, weights9};
  int i, j, k;
  int count = 0;
  while(1)
  {
    if((*(KEY_ptr + 0x3) & 0x8))
    {
      //Perform linear classification and populate array
      for(i = 0; i < 10; i++)
      {
        int sum = 0;
        for(j = 0; j < 28; j++)
        {
          for(k = 0; k < 28; k++)
          {
            sum += weights[i][j][k]*(float)img[count][j][k];
          }
        }
        output[i] = sum;
      }
      int digit = max(output,10);
      *(LED_ptr) = digit;
      count++;
      *(KEY_ptr+0x3) = 0x8;  //Clear edge detect register
    }
  }
}

//Return the index of the maximum array value
int max(int* arr, int len)
{
  int max = arr[0],   // max value
      ret = 0;        // index of max value
  int i;
  for(i = 1; i < len; i++)
  {
    if(arr[i] > max)
    {
      max = arr[i];
      ret = i;
    }
  }
  return ret;
}
