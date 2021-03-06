'''
attr_handler.py

@yashbonde 20.01.2019
'''

class AttrHandler():
	'''
	Simple class file for handling the attributes
	'''
	def __init__(self):
		self.attr = []
		self.info = None

	def add_attr(self, key, value):
		setattr(self, key, value)
		self.attr.append(key)

	def add_attr_from_dict(self, key2val):
		for key in key2val:
			setattr(self, key, key2val[key])

	def get_attr_list(self):
		return self.attr

	def get_attr_value_dict(self):
		attr_val = {}
		for a_ in self.attr:
			attr_val.update({a_: getattr(self, a_)})

	def num_attr(self):
		return len(self.attr)
