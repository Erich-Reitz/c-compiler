// EXPECTED_RETURN: 30


int main() {
    float arr[10];
    int result = 0;
    for (int i = 0; i < 10; i = i + 1) {
        arr[i] = 3.0;
    }
    if (arr[2] < 4.0) {
        return 30; 
    }
    return 1;
}