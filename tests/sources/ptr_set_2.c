// EXPECTED_RETURN: 1

void set_to_one_and_two(int *set_to_one, int *set_to_two) {
    *set_to_one = 1;
    *set_to_two = 2;
}
int main() {
    int a = 3;
    int b = 4;
    set_to_one_and_two(&a, &b);
    return a;
}