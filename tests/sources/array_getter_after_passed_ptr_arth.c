// EXPECTED_RETURN: 9

int return_last_element(int *arr, int length) {
    return *(arr + length - 1); 
}

int main() {
    int arr[10];
    for (int i = 0; i < 10; i = i + 1) {
        arr[i] = i;
    }
    return return_last_element(arr, 10);
}