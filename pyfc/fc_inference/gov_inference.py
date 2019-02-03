'''
gov_inference.py

@yashbonde - 19.01.2019
'''

from .inference_base import ActionInferenceEngine

class GovInferenceEngine(ActionInferenceEngine):
    '''
    This class handles the 'government' element of the game. There are only 6 total
    actions that can be taken. A single engine is used to run through the game
    '''
    def __init__(self, init_state, action_dict, fcio):
        ActionInferenceEngine.__init__(init_state, action_dict)
        self.fcio = fcio

        self.gov_style = ['Anarchy', 'Communism', 'Democracy', 'Despotism', 'Monarchy', 'Republic']
        self.action_list = ['change_gov_Anarchy', 'change_gov_Communism', 'change_gov_Democracy',
                            'change_gov_Despotism', 'change_gov_Monarchy', 'change_gov_Republic']
        
        self.act_dict = {} # this is the dictionary sent to fcio

        self.possible_actions = action_dict

    # backend functions

    def _cvt_act_dict(self, x):
        if isinstance(x, int):
            self.act_dict[self.action_list[x]] = True
        elif isinstance(x, str):
            self.act_dict[x] = True
        else:
            raise ValueError('cannot convert requested input to action dictionary, requested {0}'.format(x))

    def _send_dict_fcio(self, x = None):
        if x and isinstance(x, dict):
            self.fcio.send_action_dict(x)
        elif x:
            raise ValueError('trying to send {0} to fcio ')
        else:
            self.fcio.send_action_dict(self.act_dict)

    def _can_take_action(self, action_id):
        return True if self.possible_actions[action_id] else False

    # frontend functions

    def init_action(self, action_dict):
        self.possible_actions = action_dict

    def get_actions(self):
        # return list
        return self.action_list

    def take_action(self, action):
        action_id = self.action_list[action]
        if self._can_take_action(action_id):
            act_dict = self._cvt_act_dict(action_id)
            self._send_dict_fcio()
            return True
        else:
            return False

    def update_actions(self, action_dict):
        self.possible_actions = action_dict

    def get_info(self):
        return self.helptext

    # anither way to use this is by directly using keys instead of action integers

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

