// EXPECTED_RETURN: 42

void leave_array_elements_the_same(int *arr) {
    for (int i = 0; i < 10; i = i + 1) {
        arr[i] = arr[i]; 
    }
}

int main() {
    int arr[10];
    for (int i = 0; i < 10; i = i + 1) {
        arr[i] = 3;
    }
    leave_array_elements_the_same(arr); 
    if (arr[0] == 3) {
        if (arr[9] == 3) {
            return 42;
        }
    }
    return 1;
}