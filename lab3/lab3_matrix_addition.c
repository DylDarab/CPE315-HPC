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

    matrixA = (double *)malloc(rowsA * colsA * sizeof(double));
    matrixB = (double *)malloc(rowsB * colsB * sizeof(double));
    result = (double *)malloc(rowsA * colsB * sizeof(double));

    printf("Reading Matrix A...\n");
    readMatrix(filePath1, matrixA, rowsA, colsA);

    printf("Reading Matrix B...\n");
    readMatrix(filePath2, matrixB, rowsB, colsB);

    printf("Multiplying Matrix...\n");

    start_time = MPI_Wtime();

    MPI_Bcast(&rowsA, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&colsA, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&rowsB, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&colsB, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&cols, 1, MPI_INT, 0, MPI_COMM_WORLD);

    MPI_Bcast(&matrixA, rowsA * colsA, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(&matrixB, rowsB * colsB, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    colsPerProcess = colsB / size;

    MPI_Request req[size - 1];
    for (int i = 1; i < size; i++)
    {
      startIndex = i * colsPerProcess;
      colsPerProcess = i == size - 1 ? cols - startIndex : colsPerProcess;

      MPI_Irecv(&result[startIndex], colsPerProcess, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, &req[i - 1]);
    }

    MPI_Waitall(size - 1, req, MPI_STATUSES_IGNORE);
  }
  else
  {
    MPI_Bcast(&rowsA, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&colsA, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&rowsB, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&colsB, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&cols, 1, MPI_INT, 0, MPI_COMM_WORLD);

    MPI_Bcast(&matrixA, rowsA * colsA, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(&matrixB, rowsB * colsB, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    colsPerProcess = cols / size;

    printf("rank: %d, process %d cols\n", rank, colsPerProcess);

    startIndex = rank * colsPerProcess;

    for (int i = startIndex; i < startIndex + colsPerProcess; i++)
    {
      multiplyMatrix(matrixA, matrixB, rowsA, rowsB, colsA, colsB, result, i);
    }

    MPI_Request req;
    MPI_Isend(&result[startIndex], colsPerProcess, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &req);
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
    for (int i = 0; i < cols; i++)
    {
      for (int j = 0; j < rows; j++)
      {
        fprintf(resultFile, "%lf ", result[count]);
        count++;
      }
      fprintf(resultFile, "\n");
    }
    fclose(resultFile);

    printf("Export result to result.txt completed\n");
  }

  MPI_Finalize();
}