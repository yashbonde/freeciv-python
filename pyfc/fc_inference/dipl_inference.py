'''
dipl_inference.py

@yashbonde - 19.01.2019
'''

from inference_base import ActionInferenceEngine

class DiplInferenceEngine(ActionInferenceEngine):
	'''
	Main Diplomacy Inference Engine. Unlike City or Unit Inference Engines we do
	not need to use various versions of this, thus this is the sole handler of 
	all the diplomacy.
	'''
	def __init__(self, fcio, init_state):
		self.fcio = fcio