#include "network.h"
#include <Magick++.h>
#include <cstdio>
#include <cstring>
#include <ctype.h>
#include <iostream>
#include <ctime>

void teach(char * fname) {
    int l = strlen(fname);
    Magick::Image img(fname);
    prepareImage(img);
    char c = runNetwork(img);
    teachNetwork(img, tolower(fname[l - 5]));
    printf("%c--%c\n", tolower(fname[l - 5]), c);
}


int main( int argc, char ** argv) {
    Magick::InitializeMagick(*argv);
    readNetwork("base");
    srand(time(0));
    int n = argc - 1;
    for (int i = 1; i < argc; i++) {
        int j = rand()%n + 1;
        teach(argv[j]);
        std::cout << argv[j] << std::endl;
    }
    writeNetwork("base");
}
