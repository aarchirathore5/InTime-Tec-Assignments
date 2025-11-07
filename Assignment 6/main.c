
#include <stdio.h>
#include <stdlib.h>
#include "vfs.h"

int main(int argc, char* argv[]) {
    int configuredBlocks = 1024;
    if (argc >= 2) {
        int parsed = atoi(argv[1]);
        if (parsed > 0) configuredBlocks = parsed;
    }

    vfsInitialize(configuredBlocks);  // function to set up vfs
    runCli();                         // function to run interactive loop (prompt + commands)
    vfsShutdown();                    // function to cleanup

    return 0;
}
