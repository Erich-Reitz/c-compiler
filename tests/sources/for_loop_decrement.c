// EXPECTED_RETURN: 42

int main() {
    int result = 0; 
    for (int i = 10; i > 5; i = i - 1) {
        if (result > 1) {
            return 42; 
        }
        result = result + 1;  
    }
    return result;
}