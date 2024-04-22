// EXPECTED_RETURN: 13

int addFive(int *n) {
    int intermediate1 = *n;
    int addition1 = intermediate1 + 2;
    int intermediate2 = addition1;
    int addition2 = intermediate2 + 3;
    *n = addition2;
    return 0;
}

int main() {
    int value = 8;
    addFive(&value);

    return value;
}