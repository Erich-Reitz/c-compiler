// EXPECTED_RETURN: 28

int main() {
    int a = 5; 
    int b = 3;
    a = a + b;
    int c = a + 10; 
    int d = 10 + c; 
    return d; 
}