// Tony Harrington, Jadiel Arana
// This program applies a Linear Classification to hand-drawn numbers

typedef float (*weight)[28];
typedef char  (*img_type)[28];

int max(int*, int); //parameters are array of ints and length of array

int classify(img_type image)
{
  //Create array of 10 potential digits
  int output[10];
  //Initialize weights
  weight weights[10] = {weights0, weights1, weights2, weights3, weights4, weights5, weights6, weights7, weights8, weights9};
  int i, j, k;
  //Perform linear classification and populate array
  for(i = 0; i < 10; i++)
  {
    int sum = 0;
    for(k = 0; k < 28; k++)
    {
      for(j = 0; j < 28; j++)
      {
        sum += weights[i][j][k]*(float)image[j][k];
      }
    }
    output[i] = sum;
  }
  int digit = max(output,10);
  return digit;
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
