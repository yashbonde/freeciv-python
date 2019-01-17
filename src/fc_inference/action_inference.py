'''
handlers.py

This file has the various classes handling the inference engines.
'''

from map_inference import MapInferenceEngine
from city_inference import CityInferenceEngine

class UnitHandler(BaseHandler):
	def __init__(self):
		self.unit_engines = None
		self.auto = None

	def get_units_list(self):
		return self.unit_engines

	def get_auto_units(self):
		return self.unit_engines[self.auto == True]

	def get_non_auto_units(self):
		return self.unit_engines[self.auto == False]

	def get_unit_by_key(self):
		if key >= len(self.unit_engines):
			return self.unit_engines
		return list(self.unit_engines[key])

class CityHandler(BaseHandler):
	def __init__(self, cities_dict):
		self.infer_cities(cities_dict)

	def infer_cities(self, cities_dict):
		self.num_cities = len(cities_dict)
		self.city_engines = []
		for _ in self.num_cities:
			new_engine = CityInferenceEngine(**)
			self.city_engines.append(new_engine)

	def get_cities(self):
		return self.city_engines

	def get_city_by_key(self, key):
		if 0 > key >= self.num_cities:
			return self.city_engines

		return list(self.city_engines[key])

	def build_new_city(self, **kwargs):
		new_engine = CityInferenceEngine(**kwargs)
		self.city_engines.append(new_engine)
		self.num_cities += 1

	def city_got_taken(self, key):
		'''
		This is the case where the city has been taken from the user 
		and now must be removed from the the cities list
		Args:
			key: (int) ID of the cit to be removed
		'''
		self.num_cities -= 1
		
		self.city_engines.remove(key)

