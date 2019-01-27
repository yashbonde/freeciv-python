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

    world.load_from_config('./scenarios/sample.fccfg')

    world.new_game(username = 'yashbonde',
        server_ip = '127.0.0.1',
        port = 5000,
        ruleset = 'Classic',
        topology = 'ISO',
        aifill = 2,
        xsize = 16,
        ysize = 16)

    # === to use minigames
    # to get a list of minigames
    print(pyfc.MiniGames.games)
    game = pyfc.MiniGames.load_minigame('BULBRESEARCH_100')
    world.new_minigame(game)

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
            action_ = AIModel.unitNetwork.take_action(state = maps, **kwargs)

            u_.take_action(action_) # take action

            state = world.update()

        # iterate over cities
        cities = world.get_cities()

        for c_ in cities:
            pos_actions_city = c_.get_possible_actions()

            # AI MODEL HERE
            action_ = AIModel.cityNetwork.take_action(state = state)

            c_.take_action(action_)

            state = world.update()

    # some other information that we would like from the world
    print(world.network_info())
    print(world.ruleset_info())
    print(world.)


