// EXPECTED_RETURN: 100


void fill_array_with_value(int *arr, int length, int value) {
    for (int i =0; i < length; i = i + 1) {
        arr[i] = value;
    }
}

int main() {
    int arr[10]  ; 
    for (int i =0; i < 10; i = i + 1) {
        arr[i] = i; 
    }
    fill_array_with_value(arr, 10, 100); 
    return arr[9]; 
}
