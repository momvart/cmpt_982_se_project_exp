void* allocate(int size) {
    return malloc(size);
}

int main() {
    int* x = allocate(sizeof(int));
    int* y = allocate(sizeof(int));

    int z = *x + *y;
}