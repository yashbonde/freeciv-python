# Socket connection to freeciv client tornado

# importing the dependencies
from connectivity_constants import CONNECTION_TIMEOUT, RESET_TIMEOUT

class ClientSocket(object):
	'''
	Base class for connection to the client using tornado
	'''
	def __init__(self, connection_timeout = 60, reset_timeout = 60):
		'''
		Args:
			connection_timeout: seconds before severing connection
			reset_timeout: seconds before reseting connection
		'''
		self.connection_timeout = connection_timeout
		self.reset_timeout = reset_timeout

		# connection
		self.socket_connection = None
		self.socket_conn = None

	def connect(self, url):
		'''
		Connect to the given input url
		Args:
			url: url to connect to
		'''
		pass

	def send(self, packet):
		'''
		Send packet to the URL
		Args:
			packet: string to send
		'''
		pass

	def close(self):
		'''
		Close the connection
		'''
		pass
