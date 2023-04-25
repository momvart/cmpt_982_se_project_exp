void swap(char **p, char **q)
{
    char *t = *p;
    *p = *q;
    *q = t;
}
int main()
{
    char *a = malloc(4);
    char *b = malloc(4);
    swap(&a, &b);
}
