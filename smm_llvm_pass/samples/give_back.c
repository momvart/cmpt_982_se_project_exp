int* read_and_return(int* num) {
    if (*num == 0) {
        return num;
    }

    return num;
}

int main() {
    int* x = malloc(sizeof(int));
    int* y = read_and_return(x);
}