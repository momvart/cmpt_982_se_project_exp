int check(int* arr, int index) {
    return arr[index] > 2;
}

void* allocate(int size) {
    return malloc(size);
}

int main() {
    int* a = allocate(20);
    int* b = allocate(20);
    int* c = allocate(20);

    int i = 2;
    int x = check(a, i) + check(b, i);
}