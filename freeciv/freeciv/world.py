'''
This is the template file for world in freeciv-python package
'''

# importing dependencies
from connectivity import web_connection

# class
class World(object):
    '''
    The world interface for the user to interact with the game, this is what the user sees and
    experiences. It is initialised with the config file the structure of which is still unclear
    whether to use modified savefile format or write a new config file system from scratch.
    
    NOTE: This is the client end and not the compelte world as such the attributes and functions 
          are designed from the client perspective

    Args:
        config_file: path to the configuration file for the game
    '''

    def __init__(self, config_file):
        # init parameters
        self.config = utils.parse_config(config_file)

        '''
        External attributes that are set in the Step 2
        '''
        self.enable_display = enable_display
        if self.enable_display:
            self.resolution = [500, 500]

        '''
        Other parameters user need not change, can also be considered the operational attributes
        '''
        self.global_turn = 0

        self.cities = []
        self.units = []


    def initialize_world(self):
        '''
        Send the stored attrbutes to the sever and it returns a boolean when it is ready. The
        exact structure of teh attributes is still to be defined, we are focusing on the ideas
        here.
        '''
        # here the core wrapper performs check if minimum operational attributes are setup
        # in case of a confllict it raises the proper error
        world_ready = web_connection.send_initialize_world_signal(self.attributes)

        return world_ready

    def start_game(self):
        '''
        This function sends information to the server to start the game and then all the initial
        steps are then taken by the server. One such step is that all the player earlier in the
        player queue have played their turns and now the it's the turn of agent.
        '''

        # boolean check if other players have played their turn
        other_player_done = web_connection.send_start_game_signal()

        # get the map
        # the maps that we obtain will be numpy array style n-D images of shape [map_x, map_y, depth]
        maps = web_connection.get_maps()

        # get the units
        units_list = web_connection.get_units()

        # if we have fog of war
        fow_maps = []
        if self.enable_fow:
            # the last map will be mask for the FoW
            for i in range(len(maps) - 1):
                maps[i] = utils.apply_fow_mask(map = maps[i], mask = maps[-1])

        # return the values
        return maps, units_list

    def step(self, unit):
        '''
        This function is similar to OpenAI gym. Take action in the world for the input unit
        '''
        web_connection.do_step_for_unit(unit)

        # check if the turn ended, or if the game ended
        # NOTE: game end is same as done in OpenAI gym
        turn_end = web_connection.get_turn_finished()
        game_end = web_connection.get_game_finished()
        
        # get the map
        # the maps that we obtain will be numpy array style n-D images of shape [map_x, map_y, depth]
        maps = web_connection.get_maps()

        # if we have fog of war
        fow_maps = []
        if self.enable_fow:
            # the last map will be mask for the FoW
            for i in range(len(maps) - 1):
                maps[i] = utils.apply_fow_mask(map = maps[i], mask = maps[-1])

        # return the values
        return maps, turn_end, game_end

    def end_game(self):
        '''
        End the game, this ends the game irrespective of whether the game is actually finished or not
        '''

        web_connection.terminate_game() # send signal to end the game
        web_connection.cleanup() # perform cleanup by deleting all the variables and parameters 

