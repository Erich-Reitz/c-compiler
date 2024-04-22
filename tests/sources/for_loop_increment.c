// EXPECTED_RETURN: 5

int main() {
    
    int result =0; 
    for (int i = 0; i < 5; i = i + 1  ) {
        result = result + 1; 
    }
    return result;
}