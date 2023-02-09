#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

void readMatrix(int row, int col, double (*matrix)[col], char *filePath)
{
  int tempRow, tempCol;
  FILE *file = fopen(filePath, "r");
  fscanf(file, "%d %d", &tempRow, &tempCol);

  for (int i = 0; i < row; i++)
  {
    for (int j = 0; j < col; j++)
    {
      fscanf(file, "%lf", &matrix[i][j]);
    }
  }
  fclose(file);
}

void printMatrix(int row, int col, double (*matrix)[col])
{
  for (int i = 0; i < row; i++)
  {
    for (int j = 0; j < col; j++)
    {
      printf("%.3lf ", matrix[i][j]);
    }
    printf("\n");
  }
}

void writeFileMatrix(int row, int col, double (*matrix)[col], char *filePath)
{
  FILE *file = fopen(filePath, "w");
  fprintf(file, "%d %d\n", row, col);

  for (int i = 0; i < row; i++)
  {
    for (int j = 0; j < col; j++)
    {
      fprintf(file, "%lf ", matrix[i][j]);
    }
    fprintf(file, "\n");
  }
  fclose(file);
}

void addMatrix(int row, int col, double (*matrix1)[col], double (*matrix2)[col], double (*result)[col])
{
  for (int i = 0; i < row; i++)
  {
    for (int j = 0; j < col; j++)
    {
      result[i][j] = matrix1[i][j] + matrix2[i][j];
    }
  }
}

int main(int argc, char **argv)
{
  char *filepath1 = argv[1];
  char *filepath2 = argv[2];

  int row, col;

  int rank, size;

  MPI_Init(&argc, &argv);

  double start_time = MPI_Wtime();

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if (rank == 0)
  {
    FILE *file = fopen(filepath1, "r");
    fscanf(file, "%d %d", &row, &col);
    fclose(file);
  }

  MPI_Bcast(&row, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&col, 1, MPI_INT, 0, MPI_COMM_WORLD);

  double matrix1[row][col], matrix2[row][col], result[row][col];

  readMatrix(row, col, matrix1, filepath1);
  readMatrix(row, col, matrix2, filepath2);

  int rowPerProcess = row / size;
  int startRow = rank * rowPerProcess;
  int endRow = startRow + rowPerProcess;

  addMatrix(rowPerProcess, col, &matrix1[startRow], &matrix2[startRow], &result[startRow]);

  if (rank == 0)
  {
    for (int i = 1; i < size; i++)
    {
      MPI_Recv(&result[i * rowPerProcess][0], rowPerProcess * col, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
  }
  else
  {
    MPI_Send(&result[startRow][0], rowPerProcess * col, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
  }

  if (rank == 0)
  {
    writeFileMatrix(row, col, result, "result.txt");
  }

  double end_time = MPI_Wtime();
  double total_time = end_time - start_time;

  printf("Time spent in MPI: %f seconds\n", total_time);
  
  MPI_Finalize();

  return 0;
}
