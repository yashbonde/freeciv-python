'''
Freeciv inference - Units

@yashbonde 15.01.2019
'''

class UnitInferenceEngine(object):
	'''
	This is a single inference engine that is loaded by the InferenceHandler. Often
	the functions of this will be called by the client directly. Whenever an action 
	is taken, we change an attribute self.action_taken. The 
	'''
	def __init__(self, fcio):
		self.fcio = fcio

		self.pos_actions = None # dict of possible_actions

	def _convert_action_to_packet(self, action):
		'''
		Convert the action to packet to be sent
		'''
		return pack


	def take_action(self, act):
		'''
		This function is called when the client takes an action. Following are the 
		steps that happen here:
			1. The act integer is mapped to actual action
			2. The information is then converted to actual string to be sent
			3. The setring is sent using the given fcio[_manager]
		'''
		action = self.pos_actions[act]
		pack = self._convert_action_to_packet(action)
		self.fcio.send_unit_action_packet(pack)

		self.fcio.new_state_waiting = True # 

	def get_possible_actions(self):
		# return the list of possible actions based on the key
		return self.pos_actions