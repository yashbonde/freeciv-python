'''
fc_iomanager.py

This file has the FCIOManager class

@yashbonde 16.01.2019
'''

# importing the dependencies
import json

# custom


class FCIOManager():
	'''
	IOM Base has the conneci
	'''
	def __init__(self):
		pass

	'''
	Functions that IOM_Base must have
	'''
	def send(self, t):
		# function to pass the input string to the server
		pass

	'''
	Ops
	'''
	def est_conn():
		# function to establish the connection to server
		pass

	'''
	gamplay functions
	'''	
	def get_state(self):
		# return the JSON parsed state dict
		return json.load(self.state_string)