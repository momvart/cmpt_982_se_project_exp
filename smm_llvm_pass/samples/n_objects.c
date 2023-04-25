void swap(void **p, void **q)
{
    void *t = *p;
    *p = *q;
    *q = t;
}

int main() {
    void* x0 = malloc(20);
    void* x1 = malloc(20);
    void* x2 = malloc(20);
    void* x3 = malloc(20);
    
    swap(&x0, &x1);
    swap(&x2, &x3);
}