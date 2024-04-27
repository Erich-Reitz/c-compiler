// EXPECTED_RETURN: 30


int main() {
    int arr[10];
    int result = 0;
    for (int i = 0; i < 10; i = i + 1) {
        arr[i] = 3;
    }
    for (int i = 0; i < 10; i = i + 1) {
        result = result + arr[i];
    }
    return result;
}