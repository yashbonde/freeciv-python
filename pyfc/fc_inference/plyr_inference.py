'''
plyr_inference.py

@yashbonde - 19.01.2019
'''

from .inference_base import ActionInferenceEngine

class PlayerBase():
	def __init__(self, num):
		self.pid = num

	def update_attr(self, key, val):
		setattr(self, key, val)

	def update_action(self, action_):
		pass

class PlyrInferenceEngine():
	'''
	Handles all the details of the player, this is probably the most dfficult on to
	code because of it's structure. Also its behaviour is to handle diplomacy as well.

	The state information coming from them is linear, i.e. there are 546121 points
	for a game of 8 players. Whereas the actions are per player rather than linear.
	This is making writing optimized code a bif difficult.
	'''
	def __init__(self, init_state, aciton_dict, fcio):
		self.fcio = fcio

		self.name_change_dict = {'my_culture': 'culture',
			'my_current_research_cost': 'curr_research_cost',
			'my_gold': 'gold',
			'my_government': 'government',
			'my_is_alive': 'is_alive',
			'my_luxury': 'luxury',
			'my_mood': 'mood',
			'my_nation': 'nation_id',
			'my_net_income': 'net_income',
			'my_science': 'science',
			'my_science_cost': 'science_cost',
			'my_score': 'score',
			'my_revolution_finishes': 'revolution_finishes',
			'my_target_government': 'target_government',
			'my_tax': 'tax',
			'my_tech_goal': 'tech_goal',
			'my_tech_upkeep': 'tech_upkeep',
			'my_techs_researched': 'techs_researched',
			'my_total_bulbs_prod': 'total_bulbs_produced',
			'my_turns_alive': 'turns_alive'
		}

		self.opponents = None

		self.update_own_info(init_state, first = True)

	def update_state(self, state_, first = False):
		self.num_ais = state_['num_ais']
		self.humans = state_['num_humans']

		_ = [setattr(self, self.name_change_dict[k], state_[k]) for k in self.state_ if 'my_' in k]

		if first:
			pass
		# we have to add base_attributes to all the players
		for i in range(self.num_ais + self.num_humans):
			key = 'opponent_{0}'.format(i+1)
			act = [k for k in state_ if key in k]
			self.actions.append(act)

	def update_actions(self, action_):
		pass

	








