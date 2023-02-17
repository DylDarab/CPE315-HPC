#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

void readMatrix(char *filePath, double *matrix, int rows, int cols)
{
  FILE *fp;
  fp = fopen(filePath, "r");
  int count = 0;

  fscanf(fp, "%*d %*d");

  for (int i = 0; i < rows; i++)
  {
    for (int j = 0; j < cols; j++)
    {
      fscanf(fp, "%lf", &matrix[count]);
      count++;
    }
  }
}

void printArray(double *array, int size)
{
  for (int i = 0; i < size; i++)
  {
    printf("%lf ", array[i]);
  }
  printf("\n\n");
}

void multiplyMatrix(double *arrA, double *arrB, int rowsA, int rowsB, int colsA, int colsB, double *result, int valueIndex, int resultIndex)
{
  int elementRow = valueIndex / colsB;
  int elementCol = valueIndex % colsB;
  int sum = 0;

  for (int i = 0; i < colsA; i++)
  {
    sum += arrA[elementRow * colsA + i] * arrB[i * colsB + elementCol];
  }

  result[resultIndex] = sum;
}

int main(int argc, char **argv)
{
  char *filePath1 = argv[1];
  char *filePath2 = argv[2];

  int rank, size;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  double *matrixA, *matrixB, *result;
  int rowsA, colsA;
  int rowsB, colsB;
  int rows, cols;
  int colsPerProcess, startIndex = 0;

  double start_time, end_time;

  if (rank == 0)
  {
    FILE *fp;
    fp = fopen(filePath1, "r");
    fscanf(fp, "%d %d", &rowsA, &colsA);
    fclose(fp);

    fp = fopen(filePath2, "r");
    fscanf(fp, "%d %d", &rowsB, &colsB);
    fclose(fp);

    rows = rowsA;
    cols = colsB;

    printf("Matrix A => rows: %d, cols: %d\n", rowsA, colsA);
    printf("Matrix B => rows: %d, cols: %d\n", rowsB, colsB);

    printf("Result => rows: %d, cols: %d\n\n", rows, cols);

    MPI_Bcast(&rowsA, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&colsA, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&rowsB, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&colsB, 1, MPI_INT, 0, MPI_COMM_WORLD);

    matrixA = (double *)malloc(rowsA * colsA * sizeof(double));
    matrixB = (double *)malloc(rowsB * colsB * sizeof(double));
    result = (double *)malloc(rowsA * colsB * sizeof(double));

    printf("Reading Matrix A...\n");
    readMatrix(filePath1, matrixA, rowsA, colsA);

    printf("Reading Matrix B...\n");
    readMatrix(filePath2, matrixB, rowsB, colsB);

    printf("Multiplying Matrix...\n\n");

    start_time = MPI_Wtime();

    colsPerProcess = colsB / (size - 1);

    MPI_Bcast(&matrixA[0], rowsA * colsA, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(&matrixB[0], rowsB * colsB, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    MPI_Request req[size - 1];
    for (int i = 1; i < size; i++)
    {
      startIndex = (i - 1) * colsPerProcess * rows;
      if (i == size - 1)
      {
        colsPerProcess = cols - (colsPerProcess * (size - 2));
      }

      MPI_Irecv(&result[startIndex], rows * colsPerProcess, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, &req[i - 1]);
    }

    MPI_Waitall(size - 1, req, MPI_STATUSES_IGNORE);
  }
  else
  {
    MPI_Bcast(&rowsA, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&colsA, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&rowsB, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&colsB, 1, MPI_INT, 0, MPI_COMM_WORLD);

    rows = rowsA;
    cols = colsB;

    colsPerProcess = cols / (size - 1);
    startIndex = rank == 1 ? 0 : ((rank - 1) * colsPerProcess * rows);

    if (rank == size - 1)
    {
      colsPerProcess = cols - (colsPerProcess * (size - 2));
    }

    printf("rank: %d, process %d cols\n", rank, colsPerProcess);

    matrixA = (double *)malloc(rowsA * colsA * sizeof(double));
    matrixB = (double *)malloc(rowsB * colsB * sizeof(double));
    result = (double *)calloc(rowsA * colsPerProcess, sizeof(double));

    MPI_Bcast(&matrixA[0], rowsA * colsA, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(&matrixB[0], rowsB * colsB, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    for (int i = 0; i < rowsA * colsPerProcess; i++)
    {
      result[i] = 0;
    }

    for (int i = 0, j = startIndex; i < colsPerProcess * rows; i++, j++)
    {
      multiplyMatrix(matrixA, matrixB, rowsA, rowsB, colsA, colsB, result, j, i);
    }

    MPI_Request req;

    MPI_Isend(&result[0], rows * colsPerProcess, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &req);
    MPI_Wait(&req, MPI_STATUS_IGNORE);
  }

  if (rank == 0)
  {
    end_time = MPI_Wtime();
    printf("\nMultiplying Matrix Done!\n");
    printf("Total time: %lf seconds\n", end_time - start_time);

    printf("Exporting result to result.txt...\n");
    FILE *resultFile = fopen("result.txt", "w");
    int count = 0;

    fprintf(resultFile, "%d %d\r\n", rows, cols);
    for (int i = 0; i < rows; i++)
    {
      for (int j = 0; j < cols; j++)
      {
        fprintf(resultFile, "%.0lf ", result[count]);
        count++;
      }
      fprintf(resultFile, "\n");
    }
    fclose(resultFile);

    printf("Export result to result.txt completed\n");
  }

  free(matrixA);
  free(matrixB);
  free(result);

  MPI_Finalize();
}