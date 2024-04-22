#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/driver.hpp"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }
    int opt;
    while ((opt = getopt(argc, argv, "")) != -1) {
        switch (opt) {
            default:
                fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    char* sourcefile = argv[1];
    return runfile(sourcefile);
}