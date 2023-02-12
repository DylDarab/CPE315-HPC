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
    fprintf(resultFile, "%d %d\n", rows, cols);

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
    for (int i = 1; i < size; i++)
    {
      MPI_Send(&rows, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
      MPI_Send(&cols, 1, MPI_INT, i, 0, MPI_COMM_WORLD);

      if (i == size - 1)
      {
        colsPerProcess = cols - (colsPerProcess * (size - 2));
      }

      result = (double *)malloc(rows * cols * sizeof(double));

      MPI_Send(&colsPerProcess, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
      MPI_Send(&matrixA[latestIndex], colsPerProcess * rows, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
      MPI_Send(&matrixB[latestIndex], colsPerProcess * rows, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);

      MPI_Recv(&result[latestIndex], colsPerProcess * rows, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      latestIndex += colsPerProcess * rows;
    }
  }
  else
  {

    MPI_Recv(&rows, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(&cols, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(&colsPerProcess, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    printf("rank: %d, process %d cols\n", rank, colsPerProcess);

    matrixA = (double *)malloc(rows * colsPerProcess * sizeof(double));
    matrixB = (double *)malloc(rows * colsPerProcess * sizeof(double));
    result = (double *)malloc(rows * colsPerProcess * sizeof(double));

    MPI_Recv(&matrixA[0], colsPerProcess * rows, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(&matrixB[0], colsPerProcess * rows, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    addMatrix(matrixA, matrixB, result, rows, colsPerProcess);

    MPI_Send(&result[0], colsPerProcess * rows, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
  }

  if (rank == 0)
  {
    end_time = MPI_Wtime();
    printf("\nAdding Matrix Done!\n");
    printf("Total time: %lf seconds\n", end_time - start_time);

    printf("Exporting result to result.txt...\n");
    FILE *resultFile = fopen("result.txt", "w");
    int count = 0;
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
