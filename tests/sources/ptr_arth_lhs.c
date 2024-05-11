// EXPECTED_RETURN: 5


int main() {
    int arr[10]  ; 
    for (int i =0; i < 10; i = i + 1) {
        arr[i] = i; 
    }
    *(arr + 1) = 5;
    return arr[1]; 
}
