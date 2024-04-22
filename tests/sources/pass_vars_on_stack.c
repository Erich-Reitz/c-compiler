// EXPECTED_RETURN: 33

int sum7(int a, int b, int c, int d, int e, int f, int g) {
    g = 10; 
    a = 3; 
    return a + b + c + d + e + f + g;
}

int main() {
    int result = sum7(1, 2, 3, 4, 5, 6, 7);
    return result;
}