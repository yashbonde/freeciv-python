'''
inference_handler.py

This is the main inference engine and handles all the other sub classes

@yashbonde - 12.01.2019
'''

# importing the dependencies
import json

# ultra crucial thing - logging
import logging

# importing the custom handlers
from .fc_inference.city_inference import CityInferenceEngine # city
from .fc_inference.dipl_inferface import DiplInferenceEngine # dipl
from .fc_inference.gov_inference import GovInferenceEngine # gov
from .fc_inference.maps_inference import MapsInferenceEngine # maps
from .fc_inference.plyr_inference import PlyrInferenceEngine # unit
from .fc_inference.tech_inference import TechInferenceEngine # tech
from .fc_inference.unit_inference import UnitInferenceEngine # unit

# importing the non gameplay classes
from fc_inference.inference_base import NonActionInferenceEngine

# importing connectivity module
from connectivity.fc_iomanager import FCIOManager

# utils
from utils.attr_handler import attrHandler

class InferenceHandler(object):
	'''
	This is the inference handler that controls all the engines. There is no handler for inidividual
	parts such as units as was planned earlier.
	'''
	def __init__(self):

		# defining the connectivity manager
		self.fcio = FCIOManager()

		# defining attribute class
		self.ATTR = attrHandler()

		# all the units
		self.unit_engines = []
		self.unit2id = {}

		# all the cities
		self.city_engines = []

		# attributes
		self.num_moves = 0
		self.num_turns = 0
		self.MAX_TURNS = 0

	'''
	gameplay handlers
	'''

	def _add_new_unit(self, id, unit_type):
		new_unit_engine = UnitInferenceEngine(**kwargs)
		self.unit_engines.append(new_unit_engine)

	def _build_new_city(self):
		new_city_engine = CityInferenceEngine(**kwargs)
		self.city_engines.append(new_city_engine)


	def get_units_list(self, key):
		if key == 'all':
			return self.units_engines
		return self.inference_unit

	'''
	gameplay functions (ops)
	'''

	def _parse_non_action(self, state):
		'''
		Parse the non action things of state and setup inference engines for the same
		'''
		self.infr_client = NonActionInferenceEngine(state['client'])
		self.infr_game = NonActionInferenceEngine(state['game'])
		self.infr_optns = NonActionInferenceEngine(state['options'])
		self.infr_rules = NonActionInferenceEngine(state['rules'])

		# maps are special case
		self.infr_map = MapsInferenceEngine(state['map'])

	def _parse_action(self, state):
		# UNITS
		# initialise the unit engines and add them to the unit list
		for unit_id in list(state['units'].keys()):
			u_eng = UnitInferenceEngine(state['unit'][unit_id], self.fcio, unit_id)
			self.unit_engines.append(u_eng)
			self.unit2id.update({u_eng.di})

		# CITIES
		# initialize the city engines and add them to the list
		for city_id in list(state['city'].keys()):
			c_eng = CityInferenceEngine(state['city'][city_id], self.fcio, city_id)
			self.city_engines.append(c_eng)

	def est_conn(self, **kwargs):
		self.fcio.est_conn(**kwargs)
		return self.fcio.get_init_state()
		
	def start_game(self):
		'''
		This is the function that is called when the game starts. The initial state
		is parsed and the attributes are set 
		'''
		state = fcio.get_state()
		
		# parse state depending whether action or not
		self._parse_non_action(state)
		self._parse_action(state)

		return self.infr_map.get_init_map()

	def update(self):
		# function to update the game state
		self.num_moves += 1 # increment the moves
		state = self.fcio.state()

		return state

	def load_saved_game(self):
		print('inference_engine.py: load_saved_game() not implemented')


	def end_turn(self):
		self.fcio.send_end_turn_packet()

		self.num_turns += 1 # increment the turn
		return self.fcio.new_turn_init_state()