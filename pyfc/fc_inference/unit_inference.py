'''
Freeciv Inference - Units

@yashbonde - 15.01.2019
'''

# importing the dependencies
import numpy as np

# custom
from .inference_base import ActionInferenceEngine


class UnitInferenceEngine(ActionInferenceEngine):
	'''
	'''
	def __init__(self, state_init, fcio, unk_id):
		ActionInferenceEngine.__init__(state_init)
		self.fcio = fcio
		self.unk_id = unk_id

	def _convert_action_to_packet(self, action):
		'''
		Function to convert the input action into string packet information
		Args:
			action: ??

		Returns:
			pack: string to be sent to the server
		'''
		return pack

	def take_action(self, action):
		'''
		take any action
		Args:
			action: int for the actions
		'''
		self.action_mapped = self.ACTION_MAP[action]
		pack = self._convert_action_to_packet(self.action_mapped)
		self.fcio.send_packet(pack)
		self.action_taken = True

	def actions_list(self):
		return self.unit_actions

	def take_random_actions(self):
		action = np.random.randint(self.UNIT_ACTION_SPACE)
		self.take_action(action)

