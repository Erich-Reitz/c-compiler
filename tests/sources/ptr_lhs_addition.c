// EXPECTED_RETURN: 4


int main() {
    int arr[10]  ; 
    for (int i =0; i < 10; i = i + 1) {
        arr[i] = i; 
    }

    return *(arr + 4) ; 
}
