'''
reward_handler.py

The most interesting thing about the pyfc system is that you can plug any reward class
as long as it has some functions. This is similar to OpenAI gym, where any environment
can be made as long as it has some basic functions.

@yashbonde - 30.01.19
'''

class RewardHandlerDefault(object):
    '''
    This is the default reward calculator, if the user does not plug in their own reward
    handlers this is the one that is used 
    '''
    def __init__(self):
        self.cummulative_reward = 0
        self.curr_reward = None

        self.map_score = None
        self.unit_score = None
        self.city_score = None
        self.dipl_score = None
        self.plyr_score = None
        self.tech_score = None
        self.gov_score = None
        self.game_score = None

    def calculate_reward(self):
        '''
        return total reward
        '''
        return sum(self.calculate_reward_RAW())

    def calculate_reward_RAW(self):
        '''
        return list of raw rewards that are calculated
        '''
        map_rew = None
        unit_rew = None
        city_rew = None
        dipl_rew = None
        plyr_rew = None
        tech_rew = None
        gov_rew = None
        
        return [map_rew, unit_rew, city_rew, dipl_rew, plyr_rew, tech_rew, gov_rew]

    def cummulative_reward(self):
        return self.cummulative_reward

    def map_score(self):
        return self.map_score

    def unit_score(self):
        return self.unit_score

    def city_score(self):
        return self.city_score

    def dipl_score(self):
        return self.dipl_score

