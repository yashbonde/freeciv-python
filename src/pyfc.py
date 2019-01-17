'''
This is the base information Handler

@yashbonde - 12.12.19
'''

# importing Handlers
from fc_inference.city_handler import CityHandler
from fc_inference.unit_handler import UnitHandler
from fc_inference.map_handler import MapHandler



class World(object):
    '''
    This is the main game state information handler, we need to initialise this after the we
    receive the initial state information from the game, this way we can be sure to have dynamic 
    game setup. Sometimes we may have 2 players, sometimes we may have 40 players sometimes 500 
    players. Thus once we receive the first game state we establish all the Handlers and 
    InferenceEngines.

    STATE_INFORMATION_KEYS = ['city', 'client', 'dipl', 'game', 'gov', 'map', 'options',
                              'player', 'rules', 'tech', 'unit']
    '''
    def __init__(self, state):
        '''
        state: JSON parsed to dictionary
        '''
        self.is_running = False # whether the game is still operational
        

    def new_game(self, **kwargs):
        '''
        Send information to the server for the new game and let's go
        '''
        self.IOManager = IOManager

        # raw_state is the JSON parsed dictionary
        raw_state = self.IOManager.establish_connection_and_start_game(**kwargs)

        self.init_state(raw_state)

    def init_state(self, raw_state):
        # Handlers connected to the IOManager and we can do things with them
        self.CityHandler = CityHandler(state['city'])
        self.DiplomacyHandler = DiplomacyHandler(state['dipl'])
        self.GovHandler = GovHandler(state['gov'])
        self.MapHandler = MapHandler(state['map'])
        self.PlayerHandler = PlayerHandler(state['Player'])
        self.RulesHandler = RulesHandler(state['rules'])
        self.TechnologyHandler = TechnologyHandler(state['tech'])
        
        # Handlers that are not connected to the IOManager and don't do anything
        self.ClientHandler = handlers.ClientHandler(state['client'])
        self.GameHandler = handlers.GameHandler(state['game'])
        self.OptionsHandler = handlers.OptionsHandler(state['options'])
        self.UnitHandler = handlers.UnitHandler(state['unit'])

        # since all Handlers have been activated we can change the game attribute to running
        self.is_running = True

    

    '''
    The functions now are system information and they don't do anything
    when changed, they exist merely to because that is what we get
    '''
    def city_info(self):
        return self.ClientHandler.info

    def game_info(self):
        return self.GovHandler.info

    def options_info(self):
        return self.OptionsHandler.info

    def rules_info(self):
        return self.RulesHandler.info

    '''
    Operational - Game Related functions
    '''

    def end_turn(self):
        # do what happens when turn end occurs
        pass

    #### UNIT ####

    def get_units(self, type):
        if type == 'all':
            return self.UnitHandler.get_units_list()

        elif type == 'auto':
            return self.UnitHandler.get_auto_units()

        else:
            return self.UnitHandler.get_non_auto_units()

    def get_unit_by_key(self, key):
        return self.UnitHandler.get_unit_by_key(key)

    #### MAPS ####

    def get_maps

    def get_all_maps(self):
        return self.MapHandler.get_all_maps()

    def get_map_by_key(self, key = 'all'):
        if key == 'all':
            return self.get_all_maps()

        elif key == 'terrain':
            return self.MapHandler.terrain_map

        elif key ==

    '''
    Operational - Non Game Related functions
    '''

    def end_game(self):
        self.IOManager.close_game_connection()
        self.is_running = False












