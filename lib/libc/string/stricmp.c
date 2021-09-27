#include <string.h>
#include <ctype.h>
#include <sys/types.h>

int
stricmp(char const *s1, char const *s2)
{
    unsigned char c1 = '\0';
    unsigned char c2 = '\0';

    while (1) {
        c1 = *s1;
        c2 = *s2;
        s1++;
        s2++;
        if (!c1)
            break;
        if (!c2)
            break;
        if (c1 == c2)
            continue;
        c1 = tolower(c1);
        c2 = tolower(c2);
        if (c1 != c2)
            break;
    }
    return (int)c1 - (int)c2;
}
#pragma weak strcasecmp=stricmp
