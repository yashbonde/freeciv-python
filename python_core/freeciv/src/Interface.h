//
// Interface functions for the Freeciv Project
//

class interface {
public:
	interface();
	~interface();

	/* methods */
	void send(std::string);
	std::string getResponse(void);
}