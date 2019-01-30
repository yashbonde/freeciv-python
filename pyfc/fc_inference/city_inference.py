'''
city_inference.py

@yashbonde - 16.01.2019
'''

from .inference_base import ActionInferenceEngine


from pyfc.utils.debug import print_debug

# city class
class CityInferenceEngine(ActionInferenceEngine):
	def __init__(self, state_dict, fcio, unk_key):
		ActionInferenceEngine.__init__(state_dict)
		self.unk_key = unk_key
		self.fcio = fcio

		self.impr_attr = [t for t in self.attr if 'impr' in t.split('_')]
		self.man_attr = [t for t in self.attr if 'manufacture' in t.split('_')]

	'''
	backend functions
	'''
	def _convert_action_to_pack(self, key):
		action_id = CITY_ACTION_ID[key]
		

	'''
	frontend functions
	'''
	def improvements_incomplete(self):
		# get a list of all the improvements that are not completed
		print_debug(self.impr_attr, 0)
		incomp_impr = []
		for i in self.impr_attr:
			if not getattr(self, i):
				incomp_impr.append(i)
		return incomp_impr

	def improvements_complete(self):
		# get a list of all the improvements that are completed
		comp_impr = []
		for i in self.impr_attr:
			if getattr(self, i):
				comp_impr.append(i)
		return comp_impr

	def build_improvement(self, key):
		'''
		Function to set the improvement on different
		'''
		if getattr(self, key):
			print("Improvement already completed, not doing anything")
			pack = self._convert_action_to_pack('NO_OP')
			self.fcio.send_city_impr_pack(pack)

		else:
			# convert the improvement key to packet
			pack = self._convert_action_to_pack(key)
			self.fcio.send_city_impr_pack(pack)

	def manufacturing(self):
		# return a list of all the things that can be manufactured in
		# the city at the given moment
		pass