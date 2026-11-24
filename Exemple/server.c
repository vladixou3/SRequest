#include <stdio.h>
#include <stdbool.h>
#include "../SRequest.h"

int main() {

    if(!IniRequest())
        return 1;

    Request server = IniServer(6174, true);

    if(!server.socket)
        return 1;

    char buffer[4096];

    while(true) {
        if(!RecData(&server, sizeof(buffer), buffer))
            break;
    }
    Close(&server);
    return 0;
}