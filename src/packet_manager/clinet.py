'''
This is the same file as clinet.[ch] which is used for connection to the server
from client end.

@bonde - 09-01-2019
'''

# importing the dependencies

# functions

def connect_to_server(username, hostname, port, errbuf, errbufsize):
	'''
	Used to connect to the server from client
	Args:
		username: (const char *)
		hostname: (const char *)
		port: (int)
		errbuf: (char *)
		errbufsize: (int)
	'''
	pass

def make_connection(socket, username):
	'''
	Used to make connection with the server
	Args:
		socket: (int)
		username: (const char *)
	'''
	pass

def input_from_server(fd):
	'''
	Args:
		fd: (int)
	'''
	pass

def input_from_server_till_request_got_processed(fd, expected_request_id):
	'''
	Args:
		fd: (int)
		expected_request_id: (int)
	'''
	pass

def disconnect_from_server():
	'''
	Args: void
	'''
	pass

def try_to_auto_connect():
	'''
	Args: void
	'''
	pass

def start_autoconnecting_to_server():
	'''
	Args: void
	'''
	pass