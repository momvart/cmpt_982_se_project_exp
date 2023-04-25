#include <stdlib.h>

int getNum()
{
    return 10;
}

int useNum(int *x)
{
    if (*x > 0)
    {
        return 1;
    }

    return 0;
}

int *align(int **x)
{
    return &**x;
}

int main()
{
    int *x = malloc(sizeof(int));
    int *y = malloc(sizeof(int));

    int *z = getNum() % 2 == 0 ? x : y;
    useNum(align(&z));
}