// EXPECTED_RETURN: 0

int main() {
    int result = 0; 
    for (int i = 10; i > 5; i = i - 1) {
        if (result > 1) {
            return 0; 
        }
    }
    return result;
}