#include "network.h"
#include <Magick++.h>
#include <cstdio>
#include <cstring>
#include <ctype.h>


char findChar(char * s) {
    int i = strlen(s) - 1;
    while (s[i] != '/') i--;
    return s[i + 1];
}


int main( int argc, char ** argv) {
    Magick::InitializeMagick(*argv);
    readNetwork("base");
    int failed = 0, all = 0;
    for (int i = 1; i < argc; i++) {
        Magick::Image img(argv[i]);
        prepareImage(img);
        char res = runNetwork(img);
        char et = tolower(findChar(argv[i]));
        //printf("%c-%c\n", res, et);
        if (res != et) failed++;
        all++;
    }
    printf("%d/%d=%.2lf", failed, all, ((double)failed)/all);
}
