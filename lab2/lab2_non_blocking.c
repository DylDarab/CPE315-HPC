#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

void readMatrixRow(int row, int col, int columnRead, double *matrix, char *filePath)
{
  int tempRow, tempCol;
  FILE *file = fopen(filePath, "r");
  fscanf(file, "%*d %*d");

  for (int i = 0; i < row; i++)
  {
    for (int j = 0; j < col; j++)
    {
      if (i == columnRead)
      {
        fscanf(file, "%lf", &matrix[j]);
      }
      else if (i < columnRead)
      {
        fscanf(file, "%*lf");
      }
      else
      {
        break;
      }
    }
  }
  fclose(file);
}

void printRowMatrix(int row, double *matrix)
{
  for (int i = 0; i < row; i++)
  {
    printf("%lf ", matrix[i]);
  }
  printf("\n");
}

void writeRowMatrix(int row, double *matrix, FILE *file)
{
  for (int i = 0; i < row; i++)
  {
    fprintf(file, "%lf ", matrix[i]);
  }
  fprintf(file, "\n");
}

void addMatrixRow(int row, double *matrix1, double *matrix2, double *result)
{
  for (int i = 0; i < row; i++)
  {
    result[i] = matrix1[i] + matrix2[i];
  }
}

int main(int argc, char **argv)
{
  char *filepath1 = argv[1];
  char *filepath2 = argv[2];
  int row, col;
  int colPerProcess;
  int rank, size;

  double matrix1[10000], matrix2[10000], result[10000];

  MPI_Init(&argc, &argv);

  double start_time;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if (rank == 0)
  {
    start_time = MPI_Wtime();
    FILE *file = fopen(filepath1, "r");
    FILE *resultFile = fopen("./result.txt", "w");

    fscanf(file, "%d %d", &row, &col);

    printf("Row: %d, Col: %d\n", row, col);

    fprintf(resultFile, "%d %d\n", row, col);

    fclose(file);

    for (int i = 1; i < size; i++)
    {
      MPI_Send(&row, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
      MPI_Send(&col, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
    }

    for (int i = 0; i < col; i++)
    {
      readMatrixRow(row, col, i, matrix1, filepath1);
      readMatrixRow(row, col, i, matrix2, filepath2);

      MPI_Send(&matrix1[0], row, MPI_DOUBLE, (i % (size - 1)) + 1, 0, MPI_COMM_WORLD);
      MPI_Send(&matrix2[0], row, MPI_DOUBLE, (i % (size - 1)) + 1, 0, MPI_COMM_WORLD);

      MPI_Recv(&result[0], row, MPI_DOUBLE, (i % (size - 1)) + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      writeRowMatrix(row, result, resultFile);
    }
  }
  else
  {
    MPI_Recv(&row, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(&col, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    int process = (col / (size - 1));

    if ((col % (size - 1)) > (rank - 1))
    {
      process++;
    }

    printf("Process %d will process %d columns\n", rank, process);

    for (int i = 0; i < process; i++)
    {
      MPI_Recv(&matrix1[0], row, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      MPI_Recv(&matrix2[0], row, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      addMatrixRow(row, &matrix1, &matrix2, &result);

      MPI_Send(&result[0], row, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    }
  }

  if (rank == 0)
  {
    double end_time = MPI_Wtime();
    printf("\nTime: %lf seconds\n", end_time - start_time);
  }

  MPI_Finalize();
}
