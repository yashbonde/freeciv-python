'''
This is the template for Unit object.
Unit object is similar to the Unit struct in C. This high level of functionality is given to
make the agent do as much as possible to the humans can in the game. Documentation given where
needed.

Cheers!
'''

# importing the dependencies
import numpy as np

# import custom files

class Unit(object):
    '''
    Class and structure for each unit in the game, each unit has it's
    own attributes and values. All the attributes of Unit object are same
    as the one given in unit.h files, few for which I am unsure of purpose
    are kept at the bottom of __init__().
    '''

    def __init__(self, unit_type, global_turn, init_position, home_city):
        # init values
        self.unit_type = unit_type
        self.birth_turn = global_turn  # turn at which the unit was initialized
        self.position = init_position # (x,y) position
        self.home_city = home_city # intiation city

        self.owner = owner # number of owner player
        self.unit_id = int # unit_id: id

        self.moves_left = None
        self.hp = None # health
        self.veteran = None # ??
        self.fuel = None # ??

        self.DEBUG = False # bool
        self.moved = None # bool
        self.paradropped = None # bool
        
        '''
        This value is set if the unit is done moving for this turn. This
        information is used by the client.  The invariant is:
           - If the unit has no more moves, it's done moving.
           - If the unit is on a goto but is waiting, it's done moving.
           - Otherwise the unit is not done moving.
        '''
        self.done_moving = None

        self.has_orders = None

        # lists
        self.positions = []  # list of all the (x,y) coordinates it the unit has visited

        # the amount of work done on current activity. This is counted in turns
        # but is multiplied by ACTIVITY_COUNT
        self.activity_count = None

        # undefined
        self.value = int # some arbitrary value of the unit

        # not sure what these are
        self.upkeep = None # unit upkeep with regards to the homecity

        '''
        struct unit_ai ai
        enum unit_activity activity;
        struct tile *goto_tile; # May be NULL

        enum tile_special_type activity_target;
        Base_type_id           activity_base;
        enum unit_focus_status focus_status;

        int ord_map, ord_city;
        # ord_map and ord_city are the order index of this unit in tile.units
        # and city.units_supported; they are only used for save/reload 

        struct {
            int length, index;
            bool repeat;    # The path is to be repeated on completion
            bool vigilant;  # Orders should be cleared if an enemy is met
            struct unit_order *list;
        } orders;
        '''


    def take_action(self, action_val):
        '''
        The unit can take certain actions and update its attributes
        :return:
            None
        '''
        self.action = action_val

