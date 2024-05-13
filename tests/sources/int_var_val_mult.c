// EXPECTED_RETURN: 30


int main() {
    int five = 5;
    int five_teen = five * 3; 
    int five_teenalso = 3 * five;
    int thirty = five_teen + five_teenalso; 
    return thirty;
}
