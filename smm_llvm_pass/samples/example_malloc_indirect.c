void swap(char **p, char **q)
{
    char *t = *p;
    *p = *q;
    *q = t;
}

void* allocate(int size) {
    return malloc(size);
}

int main()
{
    char *a = allocate(4);
    char *b = allocate(4);
    swap(&a, &b);
}
