'''
gov_inference.py

@yashbonde - 19.01.2019
'''

from inference_base import ActionInferenceEngine

class GovInferenceEngine(ActionInferenceEngine):
	def __init__(self, fcio, **kwargs):
		self.fcio = fcio