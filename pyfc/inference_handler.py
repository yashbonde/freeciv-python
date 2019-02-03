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
from .fc_inference.dipl_inference import DiplInferenceEngine # dipl
from .fc_inference.gov_inference import GovInferenceEngine # gov
from .fc_inference.map_inference import MapInferenceEngine # maps
from .fc_inference.plyr_inference import PlyrInferenceEngine # plyr
from .fc_inference.tech_inference import TechInferenceEngine # tech
from .fc_inference.unit_inference import UnitInferenceEngine # unit

# importing the non gameplay classes
from .fc_inference.inference_base import NonActionInferenceEngine

# importing connectivity module
from .connectivity.fc_iomanager import FCIOManager

# utils
from .utils.attr_handler import AttrHandler

class InferenceHandler(object):
    '''
    This is the inference handler that controls all the engines. There is no
    handler for inidividual parts such as units as was planned earlier. This
    can be considered as the hub in hub and 
    '''
    def __init__(self):

        # defining the connectivity manager
        self.fcio = FCIOManager()

        # defining attribute class
        self.ATTR = AttrHandler()

        # cities
        self.cities = {}
        self.city_ids = []
        self.num_cities = 0

        # units
        self.units = {}
        self.unit_ids = []
        self.num_units = 0

        # attributes
        self.num_moves = 0
        self.num_turns = 0
        self.MAX_TURNS = 0

    '''
    gameplay handlers
    '''

    def _add_new_unit(self, unit_id, state_, action_):
        u_eng = UnitInferenceEngine(init_state = state_,
            action_dict = action_,
            unit_id = unit_id,
            fcio = self.fcio)

        self.units[unit_id] = u_eng
        self.unit_ids.append(unit_id)
        self.num_units += 1

    def _add_new_city(self, city_id, state_, action_):
        c_eng = CityInferenceEngine(init_state = state_,
            action_dict = action_,
            city_id = city_id,
            fcio = self.fcio)

        self.cities[city_id] = c_eng
        self.city_ids.append(city_id)
        self.num_cities += 1


    def get_unit_by_key(self, key):
        if key == 'all':
            return list(self.units.values())
        try:
            return list(self.unitid_to_unit[key])
        except:
            raise ValueError('requested unit {0} not available'.format(key))
    
    def get_city_by_key(self, key):
        if key == 'all':
            return list(self.cities.values())
        try:
            return list(self.cities[key])
        except:
            raise ValueError('requested city {0} not available'.format(key))

    '''
    gameplay functions
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

    def _parse_action(self, state_, action_):
        # UNITS
        # initialise the unit engines and add them to the unit list
        for unit_id in list(state['units'].keys()):
            self._add_new_unit(unit_id, state_, action_)

        # CITIES
        # initialize the city engines and add them to the list
        for city_id in list(state['city'].keys()):
            self._add_new_city(city_id, state_, action_)

        # GOVERNMENT
        # For government we need a single engine throughout the game, the possible actions are only for itself
        # thus added ['0'] key.
        self.infr_gov = GovInferenceEngine(state['gov'], pos_action['gov']['0'], self.fcio, self.unk_id)
        self.id2element[self.unk_id] = self.infr_gov
        self.unk_id += 1


        # TECHNOLOGY
        self.infr_tech = TechInferenceEngine(state['tech'], pos_action['tech']['curr_player'], self.fcio, self.unk_id)
        self.id2element[self.unk_id] = self.infr_tech
        self.unk_id += 1

    def _unit_moves_left(self):
        # function to determine if all units have played their moves
        moves_left = [unit.moves_left for unit in self.unit_engines]
        return sum(moves_left)

    '''
    Non Gameplay functions 
    '''

    def est_conn(self, **kwargs):
        self.fcio.est_conn(**kwargs)
        return self.fcio.get_init_state()
        
    def start_game(self):
        '''
        This is the function that is called when the game starts. The initial state
        is parsed and the attributes are set 
        '''
        state = fcio.get_state()
        pos_action = fcio.get_action()
        
        # parse state depending whether action or not
        self._parse_non_action(state)
        self._parse_action(state, pos_action)

        return self.infr_map.get_init_map()

    def update(self):
        # function to update the game state
        self.num_moves += 1 # increment the moves
        state_, action_ = self.fcio.state()

        '''
        Handle new state information, update the states for all units
        '''
        for u_id in state_['unit']:
            try:
                self.units[u_id].update(state_, action_)
            except:
                self._add_new_unit(u_id, state_, action_)

        for c_id in state_['city']:
            try:
                self.cities[c_id].update(state_, action_)
            except:
                self._add_new_city(c_id, state_, action_)


        return state

    def load_saved_game(self):
        print('inference_engine.py: load_saved_game() not implemented')

    def end_turn(self):
        self.fcio.send_end_turn_packet()

        self.num_turns += 1 # increment the turn
        return self.fcio.new_turn_init_state()

    def end_game(self):
        # the user can abruptly end the game if they want to
        pass

    def reset(self):
        # this is a very important function used for running large scale trainings
        # need to see how to efficiently implement this 
        pass