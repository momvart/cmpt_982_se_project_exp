void swap(void **p, void **q)
{
    void *t = *p;
    *p = *q;
    *q = t;
}

void* allocate(int size) {
    return malloc(size);
}

int main() {
    void* x0 = allocate(20);
    void* x1 = allocate(20);
    void* x2 = allocate(20);
    void* x3 = allocate(20);
    
    swap(&x0, &x1);
    swap(&x2, &x3);
}