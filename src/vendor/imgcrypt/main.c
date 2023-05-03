#include <string.h>
#include "crypt.h"

int main(int argc, char **argv)
{
    if(strstr(argv[0], "decrypt"))
    {
        return decrypt_firmare(argc, argv);
    }

    return encrypt_firmare(argc, argv);
}
