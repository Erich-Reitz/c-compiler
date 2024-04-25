// EXPECTED_RETURN: 42

int main() {
    int a = 1; 
    int b = 5; 
    if (a == 1) {
        if (b > 3) {
            return 42;
        }
        return 2;
    }
    return 1;
}