// EXPECTED_RETURN: 9

int return_last_element(int *arr) {
    for (int i = 0; i < 10; i = i + 1) {
        arr[i] = i;
    }
    return arr[9];
}

int main() {
    int arr[10];
 
    return return_last_element(arr);
}