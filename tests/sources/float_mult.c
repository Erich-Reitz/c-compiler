// EXPECTED_RETURN: 3

int main() {
    float five = 5.0;
    float five_teen = five * 3.0; 
    float five_teenalso = 3.0 * five;
    float thirty = five_teen + five_teenalso; 
    if (thirty < 29.1) {
        return 1; 
    }
    if (thirty > 30.1) {
        return 2; 
    }
    return 3;
}