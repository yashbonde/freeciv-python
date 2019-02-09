'''
tech_inference.py

@yashbonde - 19.01.2019
'''

import numpy as np

class Tech():
    def __init__(self, req):
        self.requirements = req

class TechInferenceEngine():
    '''
    The complexity of tech is making me feel that probably I should write it as a 
    seperate class, but this destroys the use of BaseClasses
    '''
    def __init__(self, init_state, init_actions, fcio, reward_handler):
        self.fcio = fcio
        self._Rewards = reward_handler

        self._vec_state = np.zeros([len(init_state)], dtype = np.float32)
        self._vec_action = np.zeros([len(init_actions)], dtype = np.float32)

        self.update(init_state, init_actions)

    def observe():
        return self._vec_state, self._vec_action

    def update_state(self, state_):
        self._vec_state[:] = list(state_.values())

    def update_action(self, action_):
        self._vec_action[:] = list(action_.values())

    def update(self, state_, action_):
        self.update_state(state_)
        self.update_action(action_)