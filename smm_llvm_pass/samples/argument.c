int test(int* x) {
    return *x + 2;
}

int getNum()
{
    return 10;
}

int main() {
    int *a = malloc(4);
    int *b = malloc(4);
    
    test(getNum() % 2 == 0 ? a : b);
}