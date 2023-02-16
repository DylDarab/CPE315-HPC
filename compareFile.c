#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    FILE *file1, *file2;
    int i, j, row1, col1, row2, col2;
    double ch1, ch2;

    // Open the first file in read mode
    file1 = fopen(argv[1], "r");
    if (file1 == NULL) {
        printf("Unable to open file %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    // Read the number of rows and columns from the first file
    fscanf(file1, "%d %d", &row1, &col1);

    // Open the second file in read mode
    file2 = fopen(argv[2], "r");
    if (file2 == NULL) {
        printf("Unable to open file %s\n", argv[2]);
        exit(EXIT_FAILURE);
    }

    // Read the number of rows and columns from the second file
    fscanf(file2, "%d %d", &row2, &col2);

    // Check if the dimensions of both matrices match
    if (row1 != row2 || col1 != col2) {
        printf("Matrices have different dimensions\n");
        exit(EXIT_FAILURE);
    }

    // Compare the contents of both matrices element by element
    for (i = 0; i < row1; i++) {
        for (j = 0; j < col1; j++) {
            fscanf(file1, "%lf", &ch1);
            fscanf(file2, "%lf", &ch2);
            if (ch1 != ch2) {
                printf("Matrices have different elements\n");
                exit(EXIT_FAILURE);
            }
        }
    }

    // If all contents of the matrices are the same
    printf("Correct Answer\n");

    // Close both files
    fclose(file1);
    fclose(file2);

    return 0;
}
