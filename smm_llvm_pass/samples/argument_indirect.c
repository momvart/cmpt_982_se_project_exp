int test(int* x) {
    return *x + 2;
}

int getNum()
{
    return 10;
}

void* allocate(int size) {
    return malloc(size);
}

int main() {
    int *a = allocate(4);
    int *b = allocate(4);
    
    test(getNum() % 2 == 0 ? a : b);
}