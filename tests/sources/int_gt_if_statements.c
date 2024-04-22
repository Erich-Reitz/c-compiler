// EXPECTED_RETURN: 42

int main() {
    int t = 5 > 0; 
    int nt = 0 > 5; 

    if (t) {
        if (nt) {
            // should not reach here
            return 10; 
        } else {
            // should reach here
            return 42; 
        }
        return 30; 
    }
    return 0; 
}