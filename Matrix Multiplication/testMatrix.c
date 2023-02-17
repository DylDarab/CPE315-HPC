#include <stdio.h>

void multiplyMatrix(int *arrA, int *arrB, int rowsA, int rowsB, int colsA, int colsB, int *result, int index)
{
  int elementRow = index / colsB;
  int elementCol = index % colsB;
  int sum = 0;

  for (int i = 0; i < colsA; i++)
  {
    sum += arrA[elementRow * colsA + i] * arrB[i * colsB + elementCol];
  }

  result[index] = sum;
}

int main()
{
  int rowsA = 2;
  int colsA = 4;

  int rowsB = 4;
  int colsB = 3;

  int rowsResult = rowsA;
  int colsResult = colsB;

  int matrixA[] = {1, 2, 3, 4, 5, 6, 7, 8};
  int matrixB[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};

  int result[rowsA * colsB];

  int count = 0;

  for (int i = 0; i < rowsA; i++)
  {
    for (int j = 0; j < colsB; j++)
    {
      multiplyMatrix(matrixA, matrixB, rowsA, rowsB, colsA, colsB, result, count);
      count++;
    }
  }

  for (int i = 0; i < rowsA; i++)
  {
    for (int j = 0; j < colsB; j++)
    {
      printf("%d ", result[i * colsB + j]);
    }
    printf("\n");
  }
}