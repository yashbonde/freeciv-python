'''
gov_inference.py

@yashbonde - 19.01.2019
'''

# imporing dependencies
import numpy as np # faster handling

# class
class GovInferenceEngine():
    '''
    This class handles the 'government' element of the game. There are only 6 total
    actions that can be taken. A single engine is used to run through the game
    '''
    def __init__(self, init_state, init_action, fcio, reward_handler):
        self.fcio = fcio
        self._Rewards = reward_handler
        
        self.list_action_names = list(init_state.keys())
        self.gov_style = ['Anarchy', 'Communism', 'Democracy', 'Despotism', 'Monarchy', 'Republic']

        self._vec_state = np.zeros([1], dtype = np.float32)
        self._vec_action = np.zeros([len(init_action['0'])], dtype = np.float32)

        self._act_dict = {}

        self.update(init_state, init_action)

    # backend functions

    def _cvt_act_dict(self, x):
        if isinstance(x, int):
            self._act_dict[self.list_action_names[x]] = True
        elif isinstance(x, str):
            self._act_dict[x] = True
        else:
            raise ValueError('cannot convert requested input to action dictionary, requested {0}'.format(x))

    def _send_dict_fcio(self, x = None):
        if x and isinstance(x, dict):
            self.fcio.send_action_dict(x)
        elif x:
            raise ValueError('trying to send {0} to fcio'.format(type(x)))
        else:
            self.fcio.send_action_dict(self._act_dict)

    def _can_take_action(self, action_id):
        return True if self.possible_actions[action_id] else False

    ## frontend functions

    # state
    def update_state(self, state_):
        for k,v in state_.items():
            setattr(self, k, v)
        self._vec_state[0] = state_['id']

    def observe(self):
        state = np.expand_dims(self._vec_state, axis = 0)
        action_mask = np.expand_dims(self._vec_action, axis = 0)
        return state, action_mask

    # action
    def update_action(self, action_):
        self._vec_action[:] = list(action_.values())

    def action_names(self):
        # return list
        return self.list_action_names

    def take_action(self, action):
        raise NotImplementedError('unknown packet structure for Government')
        if self._vec_action[action]:
            act_dict = self._cvt_act_dict(action_id)
            self._send_dict_fcio()
            return True
        else:
            return False


    def update(self, state_, action_):
        self.update_state(state_)
        self.update_action(action_['0']) # '0' is action key for us

    # another way to use this is by directly using keys instead of action integers

    def get_gov_type(self):
        return self.gov_style

    def change_government_type(self, key):
        # key is a string of the action
        key = key.capitalize()
        if key not in self.gov_style:
            raise ValueError('requested key \'{0}\' not posiible government styles {1}'.format(key, self.gov_style))
        cmd = 'change_gov_{}'.format(key)
        if self._can_take_action(cmd):
            act_dict = self._cvt_act_dict()
            self._send_dict_fcio()
            return True
        else:
            return False
