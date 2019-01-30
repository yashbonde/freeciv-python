'''
debug.py

These are some of the debugging tools that I have developed for pyfc

@yashbonde - 29.01.2019
'''

def print_debug(text_, severity = 0):
	'''
	Severity is defined as follows:
		0: normal debug message
		1: strong debug message [*]
		2: in progress [%]
		3: warning [!]
	'''
	if severity == 0:
		print(text_)
	elif severity == 1:
		print('[*] ' + text_)
	elif severity == 2:
		print('[%] ' + text_)
	else:
		print('[!] ' + text_)


