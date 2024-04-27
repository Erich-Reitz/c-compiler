// EXPECTED_RETURN: 5


void fill_array_with_five(int *arr, int length) {
    for (int i =0; i < length; i = i + 1) {
        arr[i] = 5;
    }
}

int main() {
    int arr[10]  ; 
    for (int i =0; i < 10; i = i + 1) {
        arr[i] = i; 
    }
    fill_array_with_five(arr, 10); 
    return arr[4]; 
}
