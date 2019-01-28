'''
city_inference.py

@yashbonde - 16.01.2019
'''

from .inference_base import ActionInferenceEngine


# city class
class CityInferenceEngine(ActionInferenceEngine):
	def __init__(self, state_dict, fcio, unk_key):
		ActionInferenceEngine(state_dict)
		self.unk_key = unk_key
		self.fcio = fcio

		self.impr_attr = [t for t in self.attr if 'impr' in t.split('_')]

	'''
	backend functions
	'''
	def _convert_action_to_pack(self, key):
		pass

	'''
	frontend functions
	'''
	def get_improvements_incomplete(self):
		# get a list of all the improvements that are not completed
		comp_impr = [i in impr if not getattr(self, i)]
		return comp_impr

	def get_improvements_complete(self):
		# get a list of all the improvements that are completed
		comp_impr = [i in impr if getattr(self, i)]
		return comp_impr

	def set_improvement(self, key):
		'''
		Function to set the improvement on different
		'''
		comp_impr = [i in impr if getattr(self, i)]
		if key in comp_impr:
			print("Improvement already completed, not doing anything")

		else:
			# convert the improvement key to packet
			pack = self._convert_action_to_pack(key)
			self.fcio.send_city_impr_pack(pack)

	def get_manufacturing(self):
		# return a list of all the things that can be manufactured in
		# the city at the given moment
		pass