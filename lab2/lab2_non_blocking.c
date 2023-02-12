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

void addMatrix(double *matrixA, double *matrixB, double *result, int rows, int cols)
{
  int count = 0;
  for (int i = 0; i < rows; i++)
  {
    for (int j = 0; j < cols; j++)
    {
      result[count] = matrixA[count] + matrixB[count];
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

int main(int argc, char **argv)
{
  char *filePath1 = argv[1];
  char *filePath2 = argv[2];

  int rank, size;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  double *matrixA, *matrixB, *result;
  int rows, cols, colsPerProcess;

  double start_time, end_time;

  if (rank == 0)
  {
    FILE *resultFile = fopen("result.txt", "w");
    FILE *fileA = fopen(filePath1, "r");
    fscanf(fileA, "%d %d", &rows, &cols);

    printf("Reading Matrix A...\n");

    matrixA = (double *)malloc(rows * cols * sizeof(double));
    readMatrix(filePath1, matrixA, rows, cols);

    printf("Reading Matrix B...\n");
    matrixB = (double *)malloc(rows * cols * sizeof(double));
    readMatrix(filePath2, matrixB, rows, cols);

    printf("Adding Matrix...\n");

    start_time = MPI_Wtime();

    colsPerProcess = cols / (size - 1);
    int latestIndex = 0;
    int reqCount = 0;
    MPI_Request reqs[size][1000];
    MPI_Request reqs2[1000];
    for (int i = 1; i < size; i++)
    {
      reqCount = 0;
      MPI_Isend(&rows, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &reqs[i][reqCount++]);
      MPI_Isend(&cols, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &reqs[i][reqCount++]);

      if (i == size - 1)
      {
        colsPerProcess = cols - (colsPerProcess * (size - 2));
      }
      else
      {
        colsPerProcess = cols / (size - 1);
      }

      MPI_Isend(&colsPerProcess, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &reqs[i][reqCount++]);
      MPI_Isend(&matrixA[latestIndex], colsPerProcess * rows, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, &reqs[i][reqCount++]);
      MPI_Isend(&matrixB[latestIndex], colsPerProcess * rows, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, &reqs[i][reqCount++]);

      latestIndex += colsPerProcess * rows;
    }

    for (int i = 1; i < size; i++)
    {
      for (int j = 0; j < reqCount; j++)
      {
        MPI_Wait(&reqs[i][j], MPI_STATUS_IGNORE);
      }
    }

    result = (double *)malloc(rows * cols * sizeof(double));

    reqCount = 0;
    latestIndex = 0;
    for (int i = 1; i < size; i++)
    {
      if (i == size - 1)
      {
        colsPerProcess = cols - (colsPerProcess * (size - 2));
      }
      else
      {
        colsPerProcess = cols / (size - 1);
      }

      MPI_Irecv(&result[latestIndex], colsPerProcess * rows, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, &reqs2[reqCount++]);

      latestIndex += colsPerProcess * rows;
    }
    for (int i = 0; i < reqCount; i++)
    {
      MPI_Wait(&reqs2[i], MPI_STATUS_IGNORE);
    }
  }
  else
  {
    MPI_Request reqs[6];

    MPI_Irecv(&rows, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &reqs[0]);
    MPI_Irecv(&cols, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &reqs[1]);
    MPI_Irecv(&colsPerProcess, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &reqs[2]);

    MPI_Wait(&reqs[0], MPI_STATUS_IGNORE);
    MPI_Wait(&reqs[1], MPI_STATUS_IGNORE);
    MPI_Wait(&reqs[2], MPI_STATUS_IGNORE);

    printf("rank: %d, process %d cols\n", rank, colsPerProcess);

    matrixA = (double *)malloc(rows * colsPerProcess * sizeof(double));
    matrixB = (double *)malloc(rows * colsPerProcess * sizeof(double));

    MPI_Irecv(matrixA, rows * colsPerProcess, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &reqs[3]);

    MPI_Irecv(matrixB, rows * colsPerProcess, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &reqs[4]);

    MPI_Wait(&reqs[3], MPI_STATUS_IGNORE);
    MPI_Wait(&reqs[4], MPI_STATUS_IGNORE);

    result = (double *)malloc(rows * colsPerProcess * sizeof(double));

    addMatrix(matrixA, matrixB, result, rows, colsPerProcess);

    MPI_Isend(result, rows * colsPerProcess, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &reqs[5]);

    MPI_Wait(&reqs[5], MPI_STATUS_IGNORE);
  }

  if (rank == 0)
  {
    end_time = MPI_Wtime();
    printf("\nAdding Matrix Done!\n");
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
