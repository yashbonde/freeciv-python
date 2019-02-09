'''
inference_handler.py

This is the main inference engine and handles all the other sub classes

Currently this is in pre-alpha MVP Stage: for now we are only interested in
showing that this thing can work. For this following functionalities has not
been added yet:

* getting unit or city by key
* end turning (turn_done variables)
* actual connection module to freeciv (currently we are using dummy_handler
    which reads replay files)
# full scale reward handling (right now we are using Default with tentacles
    in different inference engine and those infrEngines predict rewards)

@yashbonde - 12.01.2019
'''

# importing the dependencies
import json

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
    def __init__(self, reward_handler):
        '''
        Args:
            reward_handler: reward class given by world, this is top down
                class similar to how we use FCIOManager initialized here
        '''
        self.fcio = FCIOManager() # defining the connectivity manager
        self.ATTR = AttrHandler() # defining attribute class
        self._Reward = reward_handler

        # cities
        self.cities = {}
        self.city_ids = []
        self.num_cities = 0

        # units
        self.units = {}
        self.unit_ids = []
        self.num_units = 0

        # attributes
        self.move_count = 0
        self.turn_count = 0

        # set the engines
        self.infr_client = None
        self.infr_game = None
        self.infr_optns = None
        self.infr_rules = None
        self.infr_gov = None
        self.infr_tech = None
        self.infr_plyr = None
        self.infr_map = None
        self.infr_dipl = None


    '''
    BACKEND FUNCTIONS FOR GAMEPLAY
    ==============================

    Following functions handle the gameplay elements such as adding or removing
    cities or units
    '''

    def _add_new_unit(self, unit_id, state_, action_):
        u_eng = UnitInferenceEngine(init_state = state_[unit_id],
            init_action = action_[unit_id],
            unit_id = unit_id,
            fcio = self.fcio)

        self.units[unit_id] = u_eng
        self.unit_ids.append(unit_id)
        self.num_units += 1

    def _remove_unit(self, unit_id):
        # this is called when unit gets destroyed/taken
        del self.units[unit_id]
        self.unit_ids.remove(unit_id)

    def _add_new_city(self, city_id, state_, action_):
        c_eng = CityInferenceEngine(init_state = state_[city_id],
            init_action = action_[unit_id],
            city_id = city_id,
            fcio = self.fcio)

        self.cities[city_id] = c_eng
        self.city_ids.append(city_id)
        self.num_cities += 1

    def _remove_city(self, city_id):
        # this is called when city gets destroyed/taken
        del self.cities[city_id]
        self.city_ids.remove(city_id)

    def _fetch_maps(self):
        # return the maps from the map inference
        return self.infr_map.observe()

    '''
    BACKEND FUNCTIONS TO PERFORM UPDATES
    ====================================

    Following functions are rquired to update the world when any move is made
    '''

    def _update_non_action(self, state):
        # update all the states for elements that do not have actions
        if not self.infr_game and not self.infr_client and not self.infr_optns and not self.infr_rules:
            self.infr_client = NonActionInferenceEngine(state['client'])
            self.infr_game = NonActionInferenceEngine(state['game'])
            self.infr_optns = NonActionInferenceEngine(state['options'])
            self.infr_rules = NonActionInferenceEngine(state['rules'])
            return

        self.infr_client.update(state_['client'])
        self.infr_game.update(state_['game'])
        self.infr_optns.update(state_['options'])
        self.infr_rules.update(state_['rules'])

    def _update_cities(self, state_, action_):
        cities_updated = list(state_.keys())
        # check if any cities were removed from our list
        removed_cities = list(set(cities_updated + self.city_ids) - set(cities_updated))
        _ = [self.remove_city(i) for i in removed_cities] # remove cities

        for c_id in cities_updated:
            try:
                self.cities[c_id].update(state_, action_)
            except KeyError:
                self._add_new_city(c_id, state_, action_)

    def _update_units(self, state_, action_):
        units_updated = list(state_.keys())
        # check if any units were removed from our list
        removed_units = list(set(units_updated + self.unit_ids) - set(units_updated))
        _ = [self._remove_unit(i) for i in removed_units]

        for u_id in units_updated: 
            try:
                self.units[u_id].update(state_, action_)
            except KeyError:
                self._add_new_unit(u_id, state_, action_)

    def _update_map(self, state_):
        # maps are special case
        if not self.infr_map:
            self.infr_map = MapInferenceEngine(state_, self._Reward)
            return

        self.infr_map.update(state_)

    def _update_dipl(self, state_, action_):
        # update diplomacy inference
        if not self.infr_dipl:
            infr_dipl = DiplInferenceEngine(state_, action_, self.fcio, self._Reward)
            return

        self.infr_dipl.update(state_, action_)

    def _update_gov(self, state_, action_):
        # update government inference
        if not self.infr_gov:
            infr_gov = GovInferenceEngine(state_, action_, self.fcio, self._Reward)
            return

        self.infr_gov.update(state_, action_)

    def _update_tech(self, state_, action_):
        # update technology inference
        if not self.infr_tech:
            infr_tech = TechInferenceEngine(state_, action_, self.fcio, self._Reward)
            return

        self.infr_tech.update(state_, action_)

    def _update_plyr(self, state_, action_):
        # update plyrnology inference
        if not self.infr_plyr:
            infr_plyr = PlyrInferenceEngine(state_, action_, self.fcio, self._Reward)
            return

        self.infr_plyr.update(state_, action_)

    def _update(self, state_, action_):
        # function to update the game state and actions
        self._update_units(state_['unit'], action_['unit'])
        self._update_cities(state_['city'], action_['city'])

        # update other elements
        self._update_map(state_['map'])
        self._update_dipl(state_['dipl'], action_['dipl'])
        self._update_gov(state_['gov'], action_['gov'])
        # skipping tech and player for now due to extreme complexity
        # self._update_tech(state_['tech'], action_['tech'])
        # self._update_plyr(state_['player'], action_['player'])

        # update non action classes
        self._update_non_action(state_) # SHOULD WE EVEN UPDATE IT THOUGH?

        

    '''
    FRONT-END FUNCTIONS
    ===================
    '''

    def new_game(self, username, server_ip, server_port):
        # do things that happen when new game is made
        
        # establishing connection was initially in __init__ but this looks like
        # better place for it
        self.est_conn(username, server_ip, server_port)
        
        self.is_running = True

    def est_conn(self, username, server_ip, server_port):
        self.fcio.est_conn(username, server_ip, server_port)
        
    def start_game(self):
        '''
        This is the function that is called when the game starts. The initial state
        is parsed and the attributes are set 
        '''
        game_done = self.update() # perform update it if things are not existing it will
        # automatically add them

        self.is_running = game_done

        return game_done

    def update(self):
        state_, action_, game_done = self.fcio.get_observations()
        self._update(state_, action_)
        return game_done

    def save_game(self):
        raise NotImplementedError

    def load_game(self):
        raise NotImplementedError

    def load_game_from_config(self, config):
        raise NotImplementedError

    def _end_turn(self):
        # user can end turn as per his wish
        raise NotImplementedError

    def _end_game(self):
        # the user can abruptly end the game if they want to
        raise NotImplementedError

    def _reset(self):
        # this is a very important function used for running large scale trainings
        # need to see how to efficiently implement this

        # delete different elements
        del self.infr_client, self.infr_game, self.infr_optns, self.infr_rules
        del self.infr_map, self.infr_tech, self.infr_dipl
        del self.city_ids, cities
        del self.unit_ids, units

        self.num_cities, self.num_units = 0, 0

        self.fcio.reset()
        raise NotImplementedError

