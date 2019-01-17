'''
Freeciv Inference Engine - Map Inference

@yashbonde - 12.12.2019
'''

# importing the dependencies
import numpy as np # linear algebra
import json

# class
class MapInferenceEngine(object):
	'''
	This is a special case for NonAction Classes, thus it is not inherited from NonActionInferenceEngine
	'''
	def __init__(self, state_dict):
		self.map_status = np.array(state_dict)
		pass

	def get_extras_map_from_key(self, key):
		'''
		here we know x and from the extra_name_to_key_dictionary obtain the correct key
		'''
		return self.get_extras_map(extra_name_to_key_dictionary[x])

	def get_map(self, mask = True):
		# mask 
		mask_0 = self.map_status == 0. # these are the parts that have not yet been discovered
		mask_1 = [] # these are the parts that have been discovered and are in FoW
		mask_2 = [] # these are the parts that are visible

	def update_maps(self, map):
		self.map_status = map_['status']
		self.map_terrain = map_['terrain']
		self.map_extras = map_['extras']









