// EXPECTED_RETURN: 3

void swap(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

int main() {
    int a = 5;
    int b = 3;
    swap(&a, &b);
    return a;
}