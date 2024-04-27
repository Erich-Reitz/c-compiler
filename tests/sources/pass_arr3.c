// EXPECTED_RETURN: 100


void add_hundred_to_all_array_elements(int *arr, int length) {
    for (int i =0; i < length; i = i + 1) {
        arr[i] = arr[i] + 100; 
    }
}

int main() {
    int arr[10]  ; 
    for (int i =0; i < 10; i = i + 1) {
        arr[i] = 0; 
    }
    add_hundred_to_all_array_elements(arr, 10); 
    int first_and_last_equal = arr[0] == arr[9];
    if (first_and_last_equal) {
        return arr[0]; 
    } 
    return 0; 
}
