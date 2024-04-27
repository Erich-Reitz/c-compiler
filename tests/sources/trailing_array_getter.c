// EXPECTED_RETURN: 100


int trailing_last(int *arr, int length) {
    int result = 1; 
    for (int i =0; i < length; i = i + 1) {
        result = arr[i]; 
    }
    return result;
}

int main() {
    int arr[10]  ; 
    for (int i =0; i < 10; i = i + 1) {
        arr[i] = 1;     
        if (i == 9) {
            arr[i] = 100; 
        }
    }
    return trailing_last(arr, 10);
}
