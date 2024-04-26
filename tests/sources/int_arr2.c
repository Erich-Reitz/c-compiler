// EXPECTED_RETURN: 10


int main() {
    int a[5]; 
    a[0] = 2; 
    a[2] = 5;
    a[3] = 10;  
    int b[5]; 
    b[1] = 2; 
    b[4] = 3; 
    return a[3] ; 
}
