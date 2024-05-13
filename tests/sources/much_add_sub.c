// EXPECTED_RETURN: 20

int main() {
    int a = 5; 
    int b = 1 + a;
    // c == 1
    int c = a - b;
    // c == d
    int d = c + 1; 



    int e = (3 - 5) + (3 - b) + (a + a + a) + 10 + ( d - a) - (d - a);  
    return e;
}