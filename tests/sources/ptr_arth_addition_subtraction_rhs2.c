// EXPECTED_RETURN: 12


int main() {
    int arr[20]  ; 
    for (int i =0; i < 20; i = i + 1) {
        arr[i] = i; 
    }
    int five = 5; 
    int four = 4;
    return *(five +  four - 3 + 6 + arr); 
}
