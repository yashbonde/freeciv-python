'''
Freeciv world
'''

# importing the dependencies
import numpy as np

# custom dependencies
from units import Unit  # unit object
import utils  # all util functions
import < binder >  # binder file to connect to backend


class World(object):
    '''
    The world interface for the user to interact with the game, this is what the
    user sees and experiences.
    Args:
        config_file: path to the configuration file for the game
    '''

    def __init__(self, config_file):
        # init parameters
        self.config = utils.parse_config(config_file)

        # game display parameters
        self.enable_display = enable_display
        if self.enable_display:
            self.resolution = [500, 500]

        # lists

        # log lists
        self.global_scores = []
        self.unit_list = []

        # constants
        self.global_turn = 0  # turn of the game

    def Init(self, player_position=-1):
        '''
        Initialize the game by doing the following:
        1) connecting to the server using the sockets in cpp binder
        2) sending the config information to the cpp backend
        3) start the game based on position of the player.
        Args:
            player_position: the number at which the player starts, default at last
        Return
            game_init: bool (True if connection established)
        '''
        assert player_position < self.config['num_players']
        pass

    def start_game(self):
        '''
        Start the game by doing the following:
        1) check the player stack
        2) let the player above in the stack play their turns
        3) if all the other players have played their turn return True
        Returns:
            game_started: bool (True if other players have played their turn)
        '''
        pass

    def get_states(self):
        '''
        1) obtain the states from backend
        2) parse the states to proper format
        3) send to the user
        '''
        res_map, unit_map, holding_map = <binder>.get_map()
        return res_map, unit_map, holding_map

    def get_global_score(self):
        '''
        1) get the global score of the player
        2) add to the list of scores and return
        '''
        self.global_scores.append(global_score)
        return self.global_scores[-1]

    def get_units(self):
        '''
        Returns a list of the all the units that the player holds
        :return:
            units_list: Array with Unit objects
        '''
        return self.unit_list

    def update(self, unit):
        '''
        Update the world.
        :returns:
            update_success: bool telling if the update was successful
            game_done: bool telling if the game is finished
        '''
        # perform assertion that unit is indeed object of type Unit
        assert isintance(unit, Unit)

        # call the binder and if it returns True update
        update_completed, game_done = <binder>.update(unit)
        if update_completed:
            # update the global turn
            self.global_turn += 1

        # return the booleans
        return update_completed, game_done
