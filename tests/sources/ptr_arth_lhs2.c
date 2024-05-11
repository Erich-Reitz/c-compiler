// EXPECTED_RETURN: 1


int main() {
    int arr[10]  ; 
    for (int i =0; i < 10; i = i + 1) {
        arr[i] = i; 
    }
    int one = 1; 
    int four = 4; 
    *(one + four + 2 + arr) = 1;
    return arr[7]; 
}
