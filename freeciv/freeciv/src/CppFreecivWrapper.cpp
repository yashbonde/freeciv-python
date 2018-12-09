//
// This is the core wrapper class for C++ to be called from the python module
// to run the freeciv environment. We create a class here and use that to send
// signals to the client and server. This thing is still in a very early and
// is going to be very likely dumped. Even the names are a bit weird, probably
// will replace those too.
//
// This is the code file for the main source code
//

#include "CppFreecivWrapper.h"
#include "learnerComms.h"

/* ====== PRIVATE METHODS ====== */
void CppFreecivWrapper::send_signal_to_client(std::string) {

}

void CppFreecivWrapper::send_signal_to_server(std::string) {

}

/* ====== PUBLIC METHODS ====== */
void CppFreecivWrapper::CppFreecivWrapper() {

}

void CppFreecivWrapper::~CppFreecivWrapper() {

}

/* methods */

void CppFreecivWrapper::setup_connection() {

}

/* core game methods */

bool end_game(void) {
	// return true if the game ends successfully
	if ("" != sendBuffer) {
		sendBuffer << LCP_TURNDONE << "\n";
		interface::send(sendBuffer);
		sendBuffer = "";
	}
	else {
		interface::send(LCP_TURNDONE "\n")
	}

	std::string serv_response = interface::getResponse();
	while (true == serv_response.Has("ERROR")) {
		serv_response = interface::getResponse();
	}

	if (true == serv_response.StartsWith(LCP_GAMEFINISHED)) {
		return true;
	}
	return false
}

/* getter only methods (MAP ONLY) */

void get_resource_map() {

}

/* getter/setter functions for hard constants */

int get_TURN_CAP(void) {return TURN_CAP; }
void set_TURN_CAP(int new_cap) {TURN_CAP = new_cap; }
