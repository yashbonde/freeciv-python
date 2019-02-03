'''
Freeciv Inference - Units

@yashbonde - 15.01.2019
'''

# importing the dependencies
import numpy as np

# custom
from .inference_base import ActionInferenceEngine


class UnitInferenceEngine():
    '''
    After Tech this is another element that falls and I think I need to make a 
    class for it and not use the default initialisation
    '''
    def __init__(self, init_state, action_dict, fcio, unk_id, unit_id):
       
        self.fcio = fcio
        self.unk_id = unk_id

        self.unit_id = unit_id
        self.possible_actions = action_dict
        self.action_list = list(action_dict.keys())

        # often the keys used by freeciv is difficult to understand
        self.name_change_dict = {
            'type_rule_name': 'kind',
            'type_firepower': 'firepower',
            'type_convert_time': 'convert_time'
            'type_converted_to': 'converted_to_id'
            'type_vision_radius_sq': 'vision',
            'type_hp': 'hp',
            'type_attack_strength': 'attack_strength',
            'type_build_cost': 'build_cost',
            'type_defense_strength': 'defence_strength',
            'type_move_rate': 'move_rate',
            'upkeep_food': 'food_upkeep',
            'upkeep_gold': 'gold_upkeep',
            'upkeep_shield': 'shield_upkeep'
        }

        self.update_state(init_state)

        # this is the dictionary sent to fcio
        self.act_dict = {'unit_id': self.unit_id, 'action_id': None}

    # BACKEND FUNCTIONS
    def _can_take_action(self, x):
        if isinstance(x, int):
            return self.possible_actions[x]

    def _cvt_act_dict(self, x):
        if isinstance(x, int):
            self.act_dict['action_id'] = self.action_list[x]
        elif isinstance(x, str) and x in self.action_list:
            self.act_dict['action_id'] = x
        else:
            raise ValueError('cannot convert requested input to action dictionary, requested {0}'.format(x))

    def _loc_to_goto(self, x, y):
        # convert the x and y coordinates to goto number
        pass

    # UPDATES
    def update_state(self, state_dict):
        # update state with the new state_dict
        for k, v in state_dict.items():
            if k in self.name_change_dict:
                setattr(self, self.name_change_dict[k], v)
            else:
                setattr(self, k, v)

    def update_actions(self, action_dict):
        self.act_dict = {} # reset action dictionary
        self.possible_actions = action_dict

    def update(self, state, action_dict):
        self.update_state(state)
        self.update_action(action_dict)

    # ACTIONS
    def get_possible_actions(self):
        return [i for i in self.possible_actions if i]

    def take_action(self, action):
        # take any action (int)
        if self.possible_actions[action]:
            self.action_mapped = self.ACTION_MAP[action]
            act_dict = self._cvt_act_dict(action_id)
            self._send_dict_fcio()
            self.moves_left -= 1
            return True

    def goto_loc_converter(self, x, y):
        # convert the x and y spatial locations to proper goto number
        pass


    def take_random_actions(self):
        action = np.random.randint(self.UNIT_ACTION_SPACE)
        self.take_action(action)

