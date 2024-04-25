// EXPECTED_RETURN: 25

int main() {
    int a = 1; 
    int b = 5; 
    if (a == 1) {
        if (b < 3) {
            // should not reach
            return 3;
        }
        if (a != b) {
            // should reach
            return 25;
        }
        return 2; 
    }
    return 1;
}