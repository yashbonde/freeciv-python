'''
inference_base.py

@yashbonde - 16.01.2019
'''

class ActionInferenceEngine(object):
    '''
    ActionInferenceEngine is the main engine for all the elements of game in which
    actions can be taken. It is used in the following manner:
        Cities: Each city is an engine on it's own
        Units: Each unit is an engine on it's own
        Government: Single engine throughout game
        Diplomacy: Single engine throughout game
        Player: Single engine throughout game
        Tech: Single engine throughout game

    Most of the basic functions are already listed here though they may not be 
    implemented
    '''
    def __init__(self, state_dict):
        self.attr = list(state_dict.keys())
        for key in state_dict:
            setattr(self, key, state_dict[key])

    '''
    STATE:
    Since state is a very simple dictionary we can implement most of the common
    functions here, whereas special cases will be used for each engine later
    '''

    # Some basic functions to have in each action base class
    def show_status_state(self):
        # print state attribute values
        for i, attr in enumerate(self.attr):
            t = "[{0}] {1}: {2}".format(i, attr, getattr(self, attr))
            print(t)

    def RAW_state(self):
        # return the dictionary of state for this object
        raw_state = {}
        for attr in self.attr:
            raw_state[attr] = getattr(self, attr)
        return raw_state

    def update_state(self, state_dict):
        # when move is over and another 'state' and 'action' come
        # this is used to update 'state'
        for key in state_dict:
            setattr(self, key, state_dict[key])

    '''
    ACTIONS:
    Since actions are way more dynamic than the state, they are implemented per
    engine. Some basic functions though listed here may not be used at all. The
    thing with actions is that it also has boolean value that tells whether a
    particular action can be taken or not.

    In other cases with elements such as government the action space is so small
    that intializing it with seperate functions makes it really slow.
    '''
    def init_action(self, action_dict):
        '''
        Due to the unique nature of actions in this environment compared with
        other learning environments, a seperate action initializer is given.
        In this learning environment, the 'state' and  'action' come together
        for each move and 'action' tells if what user chise can be done.
        '''
        raise NotImplementedError

    def take_action(self):
        raise NotImplementedError

    def get_actions(self):
        # should return list of names of actions
        raise NotImplementedError  

    def RAW_actions(self):
        # return the dictionary of actions for this object
        raise NotImplementedError

    def update_action(self):
        # when move is over and another 'state' and 'action' come
        # this is used to update 'action'
        raise NotImplementedError

class NonActionInferenceEngine(object):
    '''
    Some elements of the game are more like information and cannot be actually
    used for decision making or have any actions corresponding to them, thus
    a simple class to set and show status is good enough.
    '''
    def __init__(self, state_dict):
        self.attr = []
        for key in state_dict:
            setattr(self, key, state_dict[key])
            self.attr.append(key)

    def show_status(self):
        for i, attr in enumerate(self.attr):
            t = "[{0}] {1}: {2}".format(i, attr, getattr(self, attr))
            print(t)

    def RAW(self):
        raw = {}
        for attr in self.attr:
            raw[attr] = getattr(self, attr)
        return raw
