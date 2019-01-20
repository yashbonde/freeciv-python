'''
inference_base.py

@yashbonde - 16.01.2019
'''

class ActionInferenceEngine(object):
    def __init__(self, state_dict):
        self.num_attr = len(state_dict)
        self.attr = list(state_dict.keys())
        for key in state_dict:
            setattr(self, key, state_dict[key])

    def init_action(self, action_dict):
        self.num_actions = len(action_dict)
        self.action_list = []
        for key in action_dict:
            k_ = 'action_'
            setattr(self, k_, state_dict[key])
            self.action_list.append(key)

    # Some basic functions to have in each action base class

    def display_attributes(self):
        for a in self.attr:
            print('ATTRIBUTE:', a)

    def get_attributes(self):
        return self.attr

    def get_num_attributes(self):
        return self.num_attr

    def get_num_actions(self):
        return self.num_actions

    def get_actions(self):
        return self.action_list 


class NonActionInferenceEngine(object):
    def __init__(self, state_dict):
        self.num_attr = 0
        self.attr = []
        for key in state_dict:
            setattr(self, key, state_dict[key])
            
            self.num_attr += 1
            self.attr.append(key)
