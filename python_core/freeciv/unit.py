'''
Unit Class
'''

# importing the dependencies
import numpy as np

# import custom files

class Unit(object):
    '''
    Class and structure for each unit in the game, each unit has it's
    own attributes and values
    '''

    def __init__(self, type, global_turn, init_position, home_city):
        # init values
        self.unit_type = type
        self.birth_turn = global_turn  # turn at which the unit was initialized
        self.position = init_position
        self.home_city = home_city

        # other values from the struct of Units
        self.owner = owner # number of owner player
        self.unit_id = int # unit_id: id

        self.moves_left = None
        self.hp = None # health
        self.veteran = None # ??
        self.fuel = None # ??

        # lists
        self.positions = []  # list of all the (x,y) coordinates it the unit has visited

        # variables
        self.action = None

        # undefined
        self.value = int  # some arbitrary value of the unit

    def take_action(self, action_val):
        '''
        The unit can take certain actions and update its attributes
        :return:
            None
        '''
        self.action = action_val

