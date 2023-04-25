#define N 5

void swap(char **p, char **q)
{
    char *t = *p;
    *p = *q;
    *q = t;
}

int getNum()
{
    return 10;
}

int main()
{
#ifdef SINGLE_OBJ
    int matrix[N][N] = {0};
#else
    int **matrix = malloc(N * sizeof(int *));
    for (int i = 0; i < N; i++)
        matrix[i] = calloc(N, sizeof(int));

#endif
    matrix[0][0] = 131;
    int i = get_num();
    int j = get_num();
    if (matrix[i][j] > 1414)
        // printf("Found positive element\n");
        return 0;

    int* a = matrix[i];
    int* b = matrix[j];
    int *z = getNum() % 2 == 0 ? a : b;
    if (z[j] > 1414)
        return 1;
}