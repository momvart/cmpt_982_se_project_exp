#define N 5

int main()
{
#ifdef SINGLE_OBJ
    int matrix[N][N][N] = {0};
#else
    int ***matrix = malloc(N * sizeof(int **));
    for (int i = 0; i < N; i++)
    {
        matrix[i] = malloc(N * sizeof(int *));
        for (int j = 0; j < N; j++)
            matrix[i][j] = calloc(N, sizeof(int));
    }

#endif
    matrix[0][0][0] = 131;
    int i = get_num();
    int j = get_num();
    int k = get_num();
    if (matrix[i][j][k] > 1414)
        // printf("Found positive element\n");
        return 0;
}