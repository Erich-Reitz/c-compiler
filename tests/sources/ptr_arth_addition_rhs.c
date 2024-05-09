// EXPECTED_RETURN: 9


int main() {
    int arr[10]  ; 
    for (int i =0; i < 10; i = i + 1) {
        arr[i] = i; 
    }
    int five = 5; 
    int four = 4;
    return *(five + arr + four); 
}
