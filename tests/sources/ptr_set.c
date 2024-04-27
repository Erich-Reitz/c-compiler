// EXPECTED_RETURN: 5

void set_to_five(int *tobe) {
    *tobe = 5; 
}
int main() {
    int a = 3;
    set_to_five(&a); 
    return a;
}