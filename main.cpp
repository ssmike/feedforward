#include "network.h"
#include <Magick++.h>
#include <cstdio>
#include <cstring>
#include <ctype.h>
#include <iostream>
#include <ctime>
#include <algorithm>

char findChar(char * s) {
    int i = strlen(s) - 1;
    while (s[i] != '/') i--;
    return s[i + 1];
}

void teach(char * fname) {
    int l = strlen(fname);
    Magick::Image img(fname);
    prepareImage(img);
    char c = runNetwork(img);
    teachNetwork(img, tolower(findChar(fname)));
    printf("%c--%c\n", tolower(findChar(fname)), c);
}


int main( int argc, char ** argv) {
    Magick::InitializeMagick(*argv);
    readNetwork("base");
    srand(time(0));
    int n = argc - 1;
    std::random_shuffle(argv + 1, argv + argc);
    for (int i = 1; i < argc; i++) {
        //int j = rand()%n + 1;
        teach(argv[i]);
        std::cout << argv[i] << std::endl;
    }
    writeNetwork("base");
}
