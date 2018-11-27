//
// This is the core wrapper class for C++ to be called from the python module
// to run the freeciv environment. We create a class here and use that to send
// signals to the client and server. This thing is still in a very early and
// is going to be very likely dumped. Even the names are a bit weird, probably
// will replace those too.
//
// This is the header file for the main source code
//

##ifndef SYMBOL
#define 
#endif

#include <pybind11/pybind11.h>

#include "ClientSideWrapper.h"
#include "ServerWrapper.h"

class CppFrecivWrapper {

private:
	int user_id; // ID for the user

	/* Hard constants */
	int TURN_CAP = 100; // maximum number of turns that can be taken 

public:
	CppFrecivWrapper();
	~ CppFrecivWrapper();

	/* methods */
	void setup_connection();
	void send_signal_to_client(std::string);
	void send_signal_to_server(std::string);

	void get_resource_map();
	void get_unit_map();
	void 

	/* getter/setter for hard constants */
	void get_TURN_CAP();
	voin set_TURN_CAP();
}