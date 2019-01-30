'''
tech_inference.py

@yashbonde - 19.01.2019
'''

from .inference_base import ActionInferenceEngine

class TechInferenceEngine(ActionInferenceEngine):
	def __init__(self, state_init, fcio):
		ActionInferenceEngine.__init__(state_init)
		self.fcio = fcio