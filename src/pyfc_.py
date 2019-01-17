'''
pyfc.py

This is the file that will be imported by the client bot
'''

from inference_handler import InferenceHandler
import utils

class World(object):
	def __init__(self):
		'''
		This is where we initialize the main class 
		Args:
		'''

		# Core gameplay attributes - game attributes
		self.plyr_civ = None

		# Core gameplay attributes (ops) - game attributes that are for the game mech
		self.is_running = False
		self.plyr_id = None # player ID given by the server
		self.unit2key = None # dictionary converting int key to string key

		# Core back attributes
		self.masterHandler = InferenceHandler

	### BASE FUNCTIONS ####
	'''
	These are the functions that run in the standard loop. All the functions
	names should start with underscore
	'''

	def _save_game():
		'''
		This is the function that saves the game periodically
		'''
		pass

	def _set_attributes_from_ifHandler():
		# once the inference handler is done inference-ing (:P)
		pass


	## BASE FUNCTION OVERRIDERS ##
	'''
	These are the functions that can be called by the client as his wish, these are same
	as the base functions but override the 
	'''
	
	def save_game(force):
		'''
		This overrides the save loop by calling _save_game() function
		'''
		self._save_game()

	#### MASTER GAME FUNCTIONS ####
	def get_units_list():
		'''
		return list of all units
		'''
		return self.masterHandler.get_units_list(key = 'all')

	def get_unit_by_key(self, key):
		return self.masterHandler.get_units_list(key = self.unit2key[key])

	def get_maps():
		return self.masterHandler.get_maps(key = 'all')


	### MAIN FUNCTIONS (NON_GAMEPLAY)###
	'''
	These are the functions that need (not must) to be called by the client
	to run the game
	'''
	def new_game_from_config(self, path):
		'''
		Load the game from config file
		Args:
			path: path to the config file
		'''
		utils.check_format(path) # perform format check
		self.xsize, self.ysize = utils.parse_standard_attr(path)

		self.pingtime, self.pingtimeout = utils.parse_rare_attr(path)

	def new_game(username,
				 server_ip,
				 server_port,
				 ruleset = None,
				 topology = None,
				 aifill = None,
				 xsize = None,
				 ysize = None,
				 endturn = None,
				 disaster = None,
				 gold = None,
				 hut = None,
				 save_frequency = None):
		'''
		This function is called to start the new game, the parameters passed through this override the ones set using config 
		file.
		
		Args:
			username: username of the bot to register on server
			server_ip: IP address of the server
			server_port: port of the server
			ruleset: ruleset to be used
			topology: 
		'''
		assert !self.is_running # if the game is running exit this command

		if username: self.username
		
		# send the IO signal to the inference engine
		self.masterHandler.est_conn(username, server_ip, server_port, ruleset, topology, aifill, xsize, ysize)

		# check if config file has been loaded
		if not self._cfg_loaded:
			self.new_game_from_config('./scenarios/default.fccfg')

		# after all the checklists and computation change the game state to running
		self.masterHandler.start_game()
		self.is_running = True
		self.can_start = True

	def start_game():
		# to start the game that has been established
		if self.can_start == True:
			self._start_game()
		else:
			raise ValueError("System not setup for initialization. Kindly check the values!")

		# this function returns the initial map
		return self.masterHandler.start_game()

	def update():
		self.masterHandler.update()

	def end_game(save = True):
		# to end the current game and delete the data
		pass

	def show_civ_attributes(self):
		self.masterHandler.client_engine.show()

	def show_networking_options(self):
		# this is where we print the complete string with all the information about networking interfacae
		pass

	## AUXILARY FUNCTIONS ##
	'''
	These are the functions that can be called by the client during gameplay
	'''
	def get_scorecard(plyr_id = self.plyr_id):
		# get scorecard of the player by id
		pass

	def reset():
		# here check state
		# only if it is possible to reset, reset the game
		pass