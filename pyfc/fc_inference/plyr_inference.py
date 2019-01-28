'''
plyr_inference.py

@yashbonde - 19.01.2019
'''

from .inference_base import ActionInferenceEngine

class PlyrInferenceEngine(ActionInferenceEngine):
	def __init__(self, init_state, fcio):
		ActionInferenceEngine.__init__(init_state)
		self.fcio = fcio