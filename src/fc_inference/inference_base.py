'''
inference_base.py

@yashbonde 16.01.2019
'''

class ActionInferenceEngine(object):
    def __init__(self, state_dict, unk_key):
        self.num_attr = 0
        self.unk_key = unk_key
        self.attr = ['unk_key']
        for key in state_dict:
            setattr(self, key, state_dict[key])
            
            self.num_attr += 1
            self.attr.append(key)


class NonActionInferenceEngine(object):
    def __init__(self, state_dict):
        self.num_attr = 0
        self.attr = []
        for key in state_dict:
            setattr(self, key, state_dict[key])
            
            self.num_attr += 1
            self.attr.append(key)
