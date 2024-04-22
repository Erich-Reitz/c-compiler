// EXPECTED_RETURN: 3

int swap(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;

    return 0; 
}

int main() {
    int a = 5;
    int b = 3;
    swap(&a, &b);
    return a;
}