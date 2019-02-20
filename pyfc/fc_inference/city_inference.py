'''
city_inference.py

@yashbonde - 16.01.2019
'''

# dependencies
import numpy as np

# city class
class CityInferenceEngine():
    '''
    CityInferenceEngine is the control centre for each city. This is called by
    the masterHandler during the operation of the game. All the functions
    relating to a city are in here.
    '''
    def __init__(self, init_state, init_action, fcio, city_id, reward_handler):
        '''
        Args:
            state_dict: This is the initial dictionary of the state to initialise the city
            fcio: This is the IOManager class object that is given by masterHandler to take
                actions internally only
            unk_key: unique key of the city
        '''
        
        self.fcio = fcio
        self.city_id = city_id
        self._Reward = reward_handler # don't do anything with this right now 
        # for MVP will be using player scores

        self.list_action_names = list(init_action.keys())

        self.name_change_dict = {}

        self._vec_state = np.zeros([len(init_state)], dtype = np.float32)
        self._vec_action = np.zeros([len(init_action)], dtype = np.float32)

        self._act_dict = {'city_id': self.city_id, 'action_id': None}

        self.update(init_state, init_action)

        self.impr_attr = [t for t in list(init_state.keys()) if 'impr' in t.split('_')]
        self.man_attr = [t for t in list(init_state.keys()) if 'manufacture' in t.split('_')]

    # Backend function
    def _send_dict_fcio(self, x):
        self.fcio.send_dict(x)

    # Core functions

    def update_state(self, state_):
        self._vec_state[:] = list(state_.values())
        for k, v in state_.items():
            try:
                setattr(self, self.name_change_dict[k], v)
            except:
                setattr(self, k, v)

    def update_action(self, actions_):
        self._vec_action[:] = list(actions_.values())

    def update(self, state_, action_):
        self.update_state(state_)
        self.update_action(action_)

    def observe(self):
        state = np.expand_dims(self._vec_state, axis = 0)
        action_mask = np.expand_dims(self._vec_action, axis = 0)
        return state, action_mask

    def take_action(self, action):
        if self._vec_action[action]:
            self._act_dict['action_id'] = True
            self._send_dict_fcio(self._act_dict)
        else:
            raise ValueError('cannot take requested action. refer to .observe()[1] to see a mask for possible actions')

        return self._Reward.latest_reward

    def sample(self):
        return np.random.randint(len(self.list_action_names))

    def safe_sample(self):
        # return sample action from those which are possible
        safe_idx = (np.arange(len(self._vec_action)) + 1) * self._vec_action
        safe_idx = safe_idx[safe_idx > 0.]
        return int(np.random.choice(safe_idx) - 1)

    '''
    backend functions
    '''
    def _convert_action_to_pack(self, key):
        action_id = CITY_ACTION_ID[key]
        

    '''
    frontend functions
    '''
    def improvements_incomplete(self):
        # get a list of all the improvements that are not completed
        print_debug(self.impr_attr, 0)
        incomp_impr = []
        for i in self.impr_attr:
            if not getattr(self, i):
                incomp_impr.append(i)
        return incomp_impr

    def improvements_complete(self):
        # get a list of all the improvements that are completed
        comp_impr = []
        for i in self.impr_attr:
            if getattr(self, i):
                comp_impr.append(i)
        return comp_impr

    def build_improvement(self, key):
        '''
        Function to set the improvement on different
        '''
        if getattr(self, key):
            print("Improvement already completed, not doing anything")
            pack = self._convert_action_to_pack('NO_OP')
            self.fcio.send_city_impr_pack(pack)

        else:
            # convert the improvement key to packet
            pack = self._convert_action_to_pack(key)
            self.fcio.send_city_impr_pack(pack)

    def manufacturing(self):
        # return a list of all the things that can be manufactured in
        # the city at the given moment
        pass