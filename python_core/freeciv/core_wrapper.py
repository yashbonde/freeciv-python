'''
This is the template for core_wrapper.
core_wrapper is the Python Wrapper for the game. This handles all the major operations
though sub operations in various types of utils might be used. core_wrapper is a set of
funtions required to send signals to the compile *.so biary which is then sent to the
client for further operation
'''

# import the compiled binary (that is the idea as of now)
from freeciv_wrapper import CppFreecivWrapper

# all the functions

def send_initialize_world_signal(attributes):
	'''
	Send the signal to start the server all the attributes required to start the game.
	Another step we get here is that we need to parse all the atributes and then send the information
	Args:
		attributes: <still need to see what to do anout this>
	'''
	pass

def send_start_game_signal():
	'''
	Send signal to server to start the game. Before doing that we need to check if the attributes are
	enough to do so. The CPP will return us a string, if the returned string is 'CHECK_COMPLETE' then
	we respond by sending signal to start the game, else we raise proper warning.
	'''
	pass

def get_maps():
	'''
	Send corresponding signal to the client to get the information about maps, it returns a long string
	and we need to process the string to get each corresponding map. We are still unsure about the exact
	details of using strings but it sets a foundation nonetheless.
	'''
	# for each map get corresponding string
	holding_string = <>
	resources_string = <>
	units_string = <>

	# process those to get proper numpy array maps
	holding_map = map_utils.process_holding_str(holding_string)
	resources_map = map_utils.process_resources_str(resources_string)
	unit_map = map_utils.process_unit_str(units_string)

	return [holding_map, resources_map, unit_map]

def get_units():
	'''
	This is going to be the really challenging one, becuase as of yet I don't know how units will work.
	I am guessing the string style response won't work here. We need to have better understanding for
	this part to work properly. Whatever be the output has to be a list filled with Unit objects.
	'''
	pass

def do_step_for_unit(unit):
	'''
	For the given input unit the action has to be taken. The action for the unit can be found in 
	unit.action attribute for the input Unit object. Send the action and some addtional signals [??]
	to the client to proess the action.
	'''
	pass

def get_turn_finished():
	'''
	send signal to see if the turn has finished or not, return boolean.
	'''
	turn_ended = <>
	return turn_ended

def get_game_finished():
	'''
	send signal to see if the game has finished or not, return boolean.
	'''
	game_ended = <>
	return game_ended

def terminate_game():
	'''
	send signal to terminate the game to server
	'''
	pass

def cleanup():
	'''
	send signal to wrapper to clean up internal variables and constant and delete itself. This will
	improve the overall performace of the package
	'''
	pass
