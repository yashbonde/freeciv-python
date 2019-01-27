'''
minigames.py

@yashbonde - 27.01.2019
'''

# importing the dependencies
from minigame_cfg import minigameConfig

from ..utils.config_utils import read_config_file_from_path
from ..utils.attr_handler import attrHandler
from ..utils.game_utils import REQ_GAME_ATTR

class minigamesBaseClass(object):
	def __init__(self, log = None):
		self.log = log

	def _load_game_names(self):
		self.games = ['BULBRESEARCH_100, BULBRESEARCH_1000']

	def _get_config_from_name(self, name):
		'''
		return minigame config file by name
		'''
		path_cfg_file = self.game_to_path[name]
		key2val = read_config_file_from_path(path_cfg_file, REQ_GAME_ATTR)
		minigameConfig = attrHandler
		for key in key2val:
			minigameConfig.add_attr_from_dict(key2val)

		if self.log:
			self.log.add_INFO('minigamesBaseClass: self attributes set')

		return minigameConfig

	def _get_config_from_path(self, path):
		'''
		load the file from path
		'''
		key2val = read_config_file_from_path(path, REQ_GAME_ATTR)
		minigameConfig = attrHandler
		for key in key2val:
			minigameConfig.add_attr_from_dict(key2val)

		if self.log:
			self.log.add_INFO('minigamesBaseClass: self attributes set')

		return minigameConfig

	def load_minigame(self, name = None, path = None):
		'''
		Function to return the config for loaded game
		Args:
			name: name of minigame
			path: path to the config file
		'''
		if not name or path:
			raise ValueError('Need to provide either name or path') 
		
		if name in self.games:
			return self._get_config_from_name(name)

		elif path:
			return self._get_config_from_path(path)

