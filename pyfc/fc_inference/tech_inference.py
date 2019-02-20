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
    This is the Technology Inference Engine. This handles the technology part of the game.
    Not implemented setattr here
    '''
    def __init__(self, init_state, init_action, fcio, reward_handler):
        self.fcio = fcio
        self._Reward = reward_handler
        self.NUM_TECH = len(init_state)
        
        self.tech_vec_attr = [
            'is_req_for_goal',
            'is_researching',
            'is_tech_goal',
            'reqs',
            'sup_improvements',
            'sup_units']

        self.name_change_dict = {
            'reqs': 'num_reqs',
            'sup_improvements': 'num_improvements',
            'sup_units': 'num_units'
        }
        
        self._make_dependency_matrix(init_state)
        self._vec_state = np.zeros([self.NUM_TECH, len(self.tech_vec_attr)], dtype = np.float32)
        self._vec_action = np.zeros([len(init_action['cur_player'])], dtype = np.float32)
        
        # self._comp_tech_vector is a vector of size 
        self._comp_tech_vector = np.zeros([len(init_state)], dtype = np.bool)

        self.update(init_state, init_action)
        
    def _make_dependency_matrix(self, state_):
        # making the dependency matrix
        self.dependency_matrix = np.zeros([len(tech), len(tech)], dtype = np.float32)
        for i,k in enumerate(tech):
            reqs = tech[k]['reqs']
            dep_nums = [int(k)-1 for k in reqs]
            self.dependency_matrix[i][dep_nums] = 1.
            
    def get_dependency_matrix(self):
        return self.dependency_matrix

    def update_state(self, state_):
        # needed to write a much more optimized code so went for nested generators
        self._vec_state[:] = [[len(tech_state[t_id][req_id]) \
                                       if isinstance(tech_state[t_id][req_id], (list, dict)) \
                                       else tech_state[t_id][req_id] \
                                   for req_id in self.tech_vec_attr] \
                              for t_id, tech in state_.items()]

    def update_action(self, action_):
        self._vec_action[:] = list(action_.values())

    def update(self, state_, action_):
        self.update_state(state_)
        self.update_action(action_['cur_player'])

    def observe(self):
        action_mask = np.expand_dims(self._vec_action, axis = 1)
        return self._vec_state, action_mask

    def take_action(self, action):
        raise NotImplementedError("[!] We don't know the required packet structure for this move")
        if self._vec_action[action]:
            pass
        else:
            return False

        self._Reward.latest_reward


    def sample(self):
        return np.random.randint(len(self._vec_action))

    def safe_sample(self):
        # return sample action from those which are possible
        safe_idx = (np.arange(len(self._vec_action)) + 1) * self._vec_action
        safe_idx = safe_idx[safe_idx > 0.]
        return int(np.random.choice(safe_idx) - 1)


