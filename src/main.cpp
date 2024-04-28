#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include <string>

#include "../include/driver.hpp"

int main(int argc, char* argv[]) {
    if (argc <= 1) {
        fprintf(stderr, "Usage: %s -o <outfile> <input file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int opt;
    std::string outfile = "test.asm";

    while ((opt = getopt(argc, argv, "o:")) != -1) {
        switch (opt) {
            case 'o':
                outfile = optarg;
                break;
            default:
                fprintf(stderr, "Usage: %s -o <outfile> <input file>\n", argv[0]);
                return EXIT_FAILURE;
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "Expected input file after options\n");
        return EXIT_FAILURE;
    }

    char* sourcefile = argv[optind];
    return runfile(sourcefile, outfile);
}
