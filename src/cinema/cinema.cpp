#include <stdio.h>
#include "../../include/ipc/communicationqueue.h"

int main() {
    printf("Cinema\n");
    commqueue test_queue = create_commqueue("/bin/bash",'a');
}
