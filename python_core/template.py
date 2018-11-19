'''
This is the template file for the idea of how the package will be run. We are focusing on
maximum ease of use and code readability for the user. This immense focus on the front-end
will automatically make us create the best possible backend.

This is just the template file, I believe that rather than writing a HOWTO file we should
document everthing as if we are the ones who are going to use it. This though will create
huge files filled with 90% comments reduces the difficulty barrier for the simple user.

This is template file version 0.0.01 the first iteration and will be used as template till
there is a better way to achieve the results. All major steps have numbers Step X: <purpose>
and all the sub-steps have format Step X.Y: <purpose>.

A similar template file has ben generated for other items in freeciv world, each with
rich documentation written from the perspective of new layman user.
'''

# import freeciv dependencies
import freeciv

'''
==== Step 1: Make the World ====

We initialise the world with config file of our format *.fcfg (freecivconfig) which
has all the majot requirements to setup a base game. It should include the following items:
    * CORE_MODE: 'default' if trainging on complete game and not minigame. This is the first thing
        that is checked and if 'default' is obtained certain attributes in the config file are
        ignored. 

    -- GLOBAL --
    Global is the list of values that are read and used even if the CORE_MODE == 'default' 
    * UNITS_AVAIL: list of units that are available to agents in the game
    * LAND_MAP: it has the map for land and water as well as the resources available at each tile
    * PLAYER_POSITION: position of the player in the 'player queue'

IDEAS HERE:
    1) Can we modify the save file structure or build supporting config file to simply use it. This
        will improve the quality of our storage and save our time building one.
    2) 

'''
world = freeciv.World('path/to/config/file.fcfg')

'''
==== Step 2: Set the Attributes ====

There are many things that will be unique the environment at the given run, such as whether the
user want to have a GUI to see the decisions that the agent is making.

FUNCATIONALITIES:
    * Something similar to gym.wrappers.monitor() to store the video of the game. This functionality
        is *very* important. I have learned that having a way to see what is happening improves the
        speed and quality of production (in that order).
    * enable_display(resolution = (tuple of x and y values), ...): show live gameplay
    * enable_fow: to enable the fog of war

IDEAS HERE:
    1) 

'''

# attributes of the world that can be changed from here
world.enable_display = False  # to have display or not
world.resolution = [100, 100]  # display resoltion
world.save_stats_folder('path/to/folder/path')
world.load_game_file('path/to/save/file')  # if the user wants to load a saved game

'''
==== Step 3: Initialize the world ====

The initialise signal is sent to the server, the python wrapper must do the checks
if all the attributes are set. If some attributes are not set then raise proper error.

IDEAS HERE:
    1)  

'''
game_init_bool = world.initialize_world()

'''
==== Step 4: Start the world ====

The start game signal is sent to the server. The client should tell server to start the game. The
server responds by starting the game. C++ wrapper must now access the game canvas (map) and get
the list of units available to the agent at that turn. The canvas is now converted to proper
strings (??, or matrices) which can be accessed by the python wrapper. The job of the python wrapper
is to convert those to numpy arrays which can be fed into the convnets as inputs. The units avaiable
list is a numpy list of all the units avaiable to the agent at that turn. The agent then iterates
over those units in Step 5 to take necessary actions.

NOTE:
    The maps returned a big list of all the images appended into a single list. If FoW is enabled
    these images are masked at the python wrapper side. These images are/can be:
        * HOLDINGS: the tiles that are controlled by different players
        * RESOURCES: the tiles that have different resources
        * UNITS: map having location of all the units
        * [??]

IDEAS HERE:
    1)  
 
''' 
maps, unit_list = world.start_game()

'''
==== Step 5: Play the World ====

The agent now has to iterate over all the units and then take actions corresponding to that unit.
For each action taken it will recieve the updated maps. Each unit in the units list is a
Unit Class object where they have their own attributes and functions that can be taken. We will
try to store as much information as possible on the python wrapper side, even if it means large
memory usage (We will see how to optimize that). The player will continue to do so till the game
ends or the number of turns ends.

IDEAS HERE:
    1) The Unit class has to be similar to the object in 
 
'''
for t in range(world.num_turns):
    # for each turn perform operations
    '''
    define the lists
    '''
    global_scores = []

    for unit in unit_list:
        # get the unit type
        unit_type = unit.unit_type()
        unit_pos = unit.latest_postion()

        '''
        use unit type and state for making the move
        action = AIModel.process_state_and_unit([maps], unit_type, unit_pos)
        '''

        # take any action for the unit
        unit.take_action(action)

        # update the world
        # done is last_turn [of the game]
        end_turn, done = world.update(unit)

        if not done:
            # get the new state
            [new_resource_map, new_units_map, new_holding_map] = world.get_state()
            new_global_score = world.get_global_score()

            # add to log
            global_scores.append(new_global_score)

            '''
            === Step 5.A: train the model ===
            AIModel.train() --> Something
            '''

            # at the end update the states
            resource_map = new_resource_map
            units_map = new_units_map
            holding_map = new_holding_map

            # if there are no more units left to take the action the turn has eneded
            # and there isn't anything left to do. world.end_turn() ends the turn
            # send server signal to do whatever it has to do.
            if end_turn:
            	world.end_turn()

        else:
            '''
            If the game is over, perform the final checks and close the game, the
            end_game() function closes the connection and tells the server to shut down.
            '''
            world.end_game()

            # if the stats are to be saved
            world.dump_stats()
