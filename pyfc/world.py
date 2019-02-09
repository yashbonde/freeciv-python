'''
world.py

This is the file that will be imported by the client bot

Currently this is in pre-alpha MVP Stage: for now we are only interested in
showing that this thing can work. For this following functionalities has not
been added yet:

* load and save game
* logging functionality
* config files (network, game)

@yashbonde - 15.01.2019
'''

from .inference_handler import InferenceHandler

# utils
from .utils.attr_handler import AttrHandler
from .utils.display_utils import show_status
from .utils.config_utils import read_config_file, write_config_file

class World(object):
    '''
    Python binding for Freeciv 3.1

    Implementation details and specs are given in READMEs
    '''
    def __init__(self, reward_handler = None):
        '''
        This is where we initialize the main class 
        Args:
            reward_handler: custom reward class file (use default)
        '''
        # main attributes
        self.moves = 0
        self.turns = 0

        # Core gameplay attributes (ops) - game attributes that are for the game mech
        self.is_running = False
        self.plyr_id = None # player ID given by the server

        # establish reward handler
        if not reward_handler:
            from .rewards.reward_handler import RewardHandlerDefault
            self.Rewards = RewardHandlerDefault()

        # Core back attributes
        self.GameATTR = AttrHandler()
        self.masterHandler = InferenceHandler(self.Rewards)

    '''
    BACKEND FUNCTIONS
    =================
    '''

    def _update(self):
        self.is_running = self.masterHandler.update()
        self.moves += 1

    def _fetch_maps(self):
        # do we need to check if their are new maps or can aleviate system
        # pressure here since we knwo that some of the actions will not
        # result in any change in the maps
        return self.masterHandler._fetch_maps()

    def _save_log(self):
        raise NotImplementedError       

    '''
    GAMEPLAY FUNCTIONS
    ==================
    '''

    def get_cities(self):
        return list(self.masterHandler.cities.values())

    def get_units(self):
        return list(self.masterHandler.units.values())


    '''
    FRONTEND FUNCTIONS
    ==================
    '''

    def new_game(self,
                 username,
                 server_ip,
                 port,
                 save_game_every = 4,
                 log_folder = './experiments/'):
        '''
        This is the functions that the user calls to start the game.
        Args:
            username: username to send to server
            server_ip: string of host server ip
            port: port to connect to host server
            save_game_every: save game every these moves
            log_folder: folder to sae the game logs in
        '''
        # set attributes
        self.save_game_every = save_game_every
        self.log_folder = log_folder

        self.masterHandler.new_game(username = username,
                server_ip = server_ip,
                server_port = port)

    def new_game_from_config(self,
                             path,
                             username = None,
                             server_ip = None,
                             port = None):
        self._load_game_config_from_path(path)

        if username:
            config.username = username

        if server_ip:
            config.server_ip = server_ip

        if port:
            config.port = port

        self.masterHandler.load_game_from_config(self.config)

    def start_game(self):
        # start the game first
        self.masterHandler.start_game()
        
        # establish connection to engines second
        self.Government = self.masterHandler.infr_gov
        self.Diplomacy = self.masterHandler.infr_dipl
        self.Technology = self.masterHandler.infr_tech
        self.Opponents = self.masterHandler.infr_plyr
        self.Client = self.masterHandler.infr_client
        self.GameInfo = self.masterHandler.infr_game
        self.Options = self.masterHandler.infr_optns
        self.Rules = self.masterHandler.infr_rules

        return self._fetch_maps()

    def take_action(self, obj, action):
        '''
        This is the final structure that I am finalising and working from now on.
        The idea is like this, the individual take_action() functions in each
        InferenceEngine will still exist but now they will be handled by World and
        not user directly.

        This way we avoid using world.update() again and again
        '''
        obj._take_action(action)
        self._update() # this has auto updates

        if self.moves % self.save_game_every == 0:
            self._save_game()

        # self._save_log()

        return self._fetch_maps() # simply return what are the latest fetched maps

    def save_game(self, path):
        self.masterHandler._save_game(path)

    def load_game(self, path):
        self.masterHandler._load_game(path)

    def reset(self):
        # reset values
        self.num_turns = 0
        self.num_moves = 0
        self.is_running = False
        self.Rewards.reset()

        # reset InferenceHandler
        self.masterHandler._reset()

        # start new game
        self.new_game()


    '''
    SHOW FUNCTIONS
    ==============
    '''

    def show_network_status(self):
        raise NotImplementedError

    def show_config_status(self):
        raise NotImplementedError