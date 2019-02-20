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
    # make world
    world = pyfc.World()
    world.new_game(username = 'LutaNet', server_ip = '127.0.0.1', port = 5004)

    # CAN ALSO INITIALIZE MINIGAME
    # print(pyfc.MiniGames.minigames)
    # minigame = pyfc.MiniGames.MAP_EXPLORER
    # world.initialize_minigame(minigame)

    # define engines
    gov = world.Government
    tech = world.Technology
    opp = world.Opponents
    dipl = world.Diplomacy

    # get initial maps
    map_terrain, map_extras, map_status = world.start_game()

    while world.is_running():
        # go over units
        for unit in world.get_units():
            while unit.can_take_action():
                # action_mask tells which actions can be taken
                obs, action_mask = unit.observe()
                action = unit.sample() # sample an action for now, put AI here
                map_status, map_terrain, map_specials, reward = world.take_action(unit, action)
                
        # go over cities
        for city in world.get_cities():
            # action_mask tells which actions can be taken
            obs, action_mask = city.observe()
            action = city.sample() # sample an action for now, put AI here
            map_status, map_terrain, map_specials, reward = world.take_action(city, action)
       
        # if want to go over other engines as well
        
        # government
        obs, action_mask = gov.observe()
        action = gov.sample() # sample an action for now, put AI here
        map_status, map_terrain, map_specials, reward = world.take_action(gov, action)
        
        # technology
        obs, action_mask = tech.observe()
        action = tech.sample() # sample an action for now, put AI here
        map_status, map_terrain, map_specials, reward = world.take_action(tech, action)
        
        # diplomacy
        obs, action_mask = dipl.observe()
        action = dipl.sample() # sample an action for now, put AI here
        map_status, map_terrain, map_specials, reward = world.take_action(dipl, action)
        
        # opponents
        for player in opponents.get_player():
            obs, action_mask = player.observe()
            action = player.sample() # sample an action for now, put AI here
            map_status, map_terrain, map_specials, reward = world.take_action(player, action) 
    
    # some other information that we would like from the world
    print(world.network_info())
    print(world.ruleset_info())

