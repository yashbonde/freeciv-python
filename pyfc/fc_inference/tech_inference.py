'''
tech_inference.py

@yashbonde - 19.01.2019
'''

from .inference_base import ActionInferenceEngine

class Tech():
    def __init__(self, req):
        self.requirements = req

class TechInferenceEngine():
    '''
    The complexity of tech is making me feel that probably I should write it as a 
    seperate class, but this destroys the use of BaseClasses
    '''
    def __init__(self, state_init, action_dict, fcio):
        self.fcio = fcio

        self.research_completed = []

    def _setup_actions(self, action_dict):
        self.actions = [k for k in list(action_dict.keys())]
        self.id2act = {int(a.split('_')[-1]) for a in self.actions}

    def get_actions(self):
        return self.actions
