#include "../SRequest.h"

int main(void) {
	if(!IniRequest()) {
		return -1;
	}
	Request request = IniConnexion("127.0.0.1", 6174, true);
	if(!SConnect(&request)) {
		return 1;
	}

	SendData("Hello SRequest!\n", &request);
	Close(&request);
	return 0;
}