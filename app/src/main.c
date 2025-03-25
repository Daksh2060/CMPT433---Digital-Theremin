
#include <stdio.h>
#include "udp_controls.h"

int main()
{
    udp_init();
    while (1) {
        sleep(1);
    }
    udp_cleanup();
    return 0;
}