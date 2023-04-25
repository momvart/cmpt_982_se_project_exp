int check(int* arr, int index) {
    return arr[index] > 2;
}

int main() {
    int* a = malloc(20);
    int* b = malloc(20);
    int* c = malloc(20);

    int i = 2;
    int x = check(a, i) + check(b, i);
}