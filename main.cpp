#include "network.h"
#include <Magick++.h>
#include <cstdio>
#include <cstring>
#include <ctype.h>
#include <iostream>

void teach(char * fname) {
    int l = strlen(fname);
    Magick::Image img(fname);
    prepareImage(img);
    teachNetwork(img, fname[l - 5]);
}


int main( int argc, char ** argv) {
    Magick::InitializeMagick(*argv);
    readNetwork("base");
    for (int i = 1; i < argc; i++) {
        teach(argv[i]);
        std::cout << argv[i] << std::endl;
    }
    writeNetwork("base");
}
