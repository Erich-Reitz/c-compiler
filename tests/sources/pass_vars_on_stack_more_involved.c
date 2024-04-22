// EXPECTED_RETURN: 65

int sum12(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j, int k, int l) {
    return a + b + c + d + e + f + g + h + i + j + k + l;
}

int sum10_plus_5_plus_5(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j) {
    return sum12(a, b,c, d, e, f, g, h, i, j, 5, 5); 
}

int main() {
    int ten = 10; 
    int eight = 8; 
    int result = sum10_plus_5_plus_5(1, 2, 3, 4, 5, 6, 7, eight, 9, ten);
    return result;
}