'''
dipl_inference.py

@yashbonde - 19.01.2019
'''

import numpy as np

class DiplInferenceEngine():
    '''
    Main Diplomacy Inference Engine. Unlike City or Unit Inference Engines we do
    not need to use various versions of this, thus this is the sole handler of 
    all the diplomacy.
    '''
    def __init__(self, init_state, init_actions, fcio, reward_handler):
        self.fcio = fcio
        self._Rewards = reward_handler

        self._vec_state = np.zeros([len(init_state)], dtype = np.float32)
        self._vec_action = np.zeros([len(init_actions)], dtype = np.float32)

        self.update(init_state, init_actions)

    def observe(self):
        state = np.expand_dims(self._vec_state, axis = 0)
        action_mask = np.expand_dims(self._vec_action, axis = 0)
        return state, action_mask

    def update_state(self, state_):
        self._vec_state[:] = list(state_.values())

    def update_action(self, action_):
        self._vec_action[:] = list(action_.values())

    def update(self, state_, action_):
        self.update_state(state_)
        self.update_action(action_)