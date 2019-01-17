'''
Sample client file

@yashbonde 16.01.2019
'''

# import env
import pyfc

# importing other dependencies...

# sample network class
class AIModel():
    def __init__(self):
        pass

# running the game
def run_game():
    world = pyfc.World()

    world.new_game_from_config('./scenarios/sample.fccfg')

    world.new_game(username = 'yashbonde',
        server_ip = '127.0.0.1',
        port = 5000,
        ruleset = 'Classic',
        topology = 'ISO',
        aifill = 2,
        xsize = 16,
        ysize = 16)

    state_map = world.start_game()

    while world.is_running():
        # get maps
        state_map = world.get_maps()

        # get units
        units = world.get_all_units(type = 'all')      # all units
        units = world.get_all_units(type = 'auto')     # units in auto mode

        # default
        units = world.get_all_units() # units being controlled (non-auto units)

        unit = world.get_unit_by_key(key = 12)

        for u_ in units:
            pos_actions = u_.get_possible_actions()

            # AI MODEL HERE
            action_ = AIModel.take_action(state = maps, **kwargs)

            u_.take_action(action_) # take action

            new_state = world.update()

        # iterate over cities
        cities = world.get_cities()

        for c_ in cities:



