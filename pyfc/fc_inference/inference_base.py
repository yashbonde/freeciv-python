'''
inference_base.py

@yashbonde - 16.01.2019
'''

class ActionInferenceEngine(object):
    def __init__(self, state_dict):
        self.attr = list(state_dict.keys())
        for key in state_dict:
            setattr(self, key, state_dict[key])

    def init_action(self, action_dict):
        self.action_list = []
        for key in action_dict:
            k_ = 'action_'
            self.action_list.append(key)

    # Some basic functions to have in each action base class
    def show_status(self):
        for i, attr in enumerate(self.attr):
            t = "[{0}] {1}: {2}".format(i, attr, getattr(self, attr))
            print(t)

    def get_actions(self):
        pass

    def RAW_state(self):
        # return the dictionary of state for this object
        pass

    def RAW_actions(self):
        # return the dictionary of actions for this object
        pass

class NonActionInferenceEngine(object):
    def __init__(self, state_dict):
        self.attr = []
        for key in state_dict:
            setattr(self, key, state_dict[key])
            self.attr.append(key)

    def show_status(self):
        for i, attr in enumerate(self.attr):
            t = "[{0}] {1}: {2}".format(i, attr, getattr(self, attr))
            print(t)
