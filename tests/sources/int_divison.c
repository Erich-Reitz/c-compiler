// EXPECTED_RETURN: 1

int main() {
    int four = 8 / 2; 
    int two = four / 2; 
    int eight = two * 4;
    int one = 8 / eight; 
    return one;
}