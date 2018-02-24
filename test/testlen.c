#include <stdio.h>

int main() {
    char *c = "Hello World!\0";

    char *s = c;
    int i = 0;
    while (*s != '\0') {
        i++;
        //s++;
    }
    printf("%d\n", i);

    return 0;
}