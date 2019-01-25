'''
display_utils.py

This has classes and functions responsible to display values

@yashbonde - 20.01.2019
'''

def show_status(name, values_dict):
	print("[#] Displaying {0} Status".format(name))
	for i, attr in enumerate(values_dict):
		t = "[{0}] {1}: {2}".format(i, attr, values_dict[attr])
		print(t)

