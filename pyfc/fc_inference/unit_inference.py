'''
Freeciv Inference - Units

@yashbonde - 15.01.2019
'''

# importing the dependencies
import numpy as np

# class
class UnitInferenceEngine():
    '''
    After Tech this is another element that falls and I think I need to make a 
    class for it and not use the default initialisation
    '''
    def __init__(self, init_state, init_action, fcio, unit_id):
       
        self.fcio = fcio
        self.unit_id = unit_id

        # often the keys used by freeciv is difficult to understand
        self.name_change_dict = {
            'type_rule_name': 'kind',
            'type_firepower': 'firepower',
            'type_convert_time': 'convert_time',
            'type_converted_to': 'converted_to_id',
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

        self.ignore_attrs = ['type_rule_name']

        self.list_actions_name = list(init_action.keys())
        
        self._vec_state = np.zeros([len(init_state) - 1], dtype = np.float32)
        self._vec_action = np.zeros([len(init_action)], dtype = np.float32)
        self.act_dict = {'unit_id': self.unit_id, 'action_id': None}

        self.update(init_state, init_action)

    def _send_dict_fcio(self, x):
        self.fcio.send_dict(x)

    def update_state(self, state_):
        # self._vec_state[:] = list(state_.values())
        counter = 0
        for k, v in state_.items():
            try:
                setattr(self, self.name_change_dict[k], v)
            except:
                # this means rest is probably fine
                setattr(self, k, v)

            if k == 'type_rule_name':
                continue

            self._vec_state[counter] = v
            counter += 1

    def observe(self):
        state = np.expand_dims(self._vec_state, axis = 0)
        action_mask = np.expand_dims(self._vec_action, axis = 0)
        return state, action_mask

    def update_actions(self, actions_):
        self._vec_action[:] = list(actions_.values())

    def take_action(self, action):
        if self._vec_action[action]:
            self._act_dict['action_id'] = self.list_actions_name[action]
            self._send_dict_fcio(self._act_dict)

            # earlier there were lines to handle the moves and all that internally now we are 
            # dependent on server to send the updates, this improves the speed of package
            # and reduces any additional load
        else:
            raise ValueError('cannot take requested action. refer to .observe()[1] to see a mask for possible actions')
    
    def update(self, state_, actions_):
        self.update_state(state_)
        self.update_actions(actions_)

    def sample(self):
        return np.random.randint(len(self._vec_action))

    def safe_sample(self):
        # return sample action from those which are possible
        return None

    def take_random_action(self):
        self.take_action(self.sample())

    # BACKEND FUNCTIONS
    def _loc_to_goto(self, x, y):
        # convert the x and y coordinates to goto number
        pass

    def goto_loc_converter(self, x, y):
        # convert the x and y spatial locations to proper goto number
        pass

