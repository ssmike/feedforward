#include "network.h"
#include <Magick++.h>
#include <cstdio>
#include <cstring>
#include <ctype.h>

int main( int argc, char ** argv) {
    Magick::InitializeMagick(*argv);
    Magick::Image img(argv[argc - 1]);
    prepareImage(img);
    readNetwork("base");
    int l = strlen(argv[argc - 1]);
    teachNetwork(img, tolower(argv[argc - 1][l - 5]));
    printf("%c--\n", runNetwork(img));
    writeNetwork("base");
}
