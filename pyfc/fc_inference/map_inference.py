'''
Freeciv Inference - Map Inference

** There is serious issue that needs to be fixed ASAP, the extras map is
currently combined into a single map and then sent to the agent. This
removes the uniqueness of extras as the system cannot differentiate between
them. And the extreme sparseness of extras map makes it difficult to be
used effectively!

@yashbonde - 12.12.2019
'''

# importing the dependencies
import numpy as np

# class
class MapInferenceEngine(object):
    '''
    Class to handle the maps
    '''
    def __init__(self, init_state, _rewards):
        self.status_map = np.array(init_state['status'], dtype = np.float32)
        self.terrain_map = np.array(init_state['terrain'], dtype = np.float32)

        self.xsize, self.ysize = self.status_map.shape[:2]
        self.num_extras = len(init_state['extras'])
        self.extra_base = np.zeros([self.xsize, self.ysize], dtype = np.float32)

        self.extras_map = self._cvt_extras_to_single_map(init_state['extras'])

        self._Rewards = _rewards

    def _get_masked_explored(self, status, base_map):
        stm = status >= 1.
        masked_map = base_map * stm
        return masked_map

    def _reset_base(self):
        self.extra_base[:] = 0

    def _cvt_extras_to_single_map(self, extras):
        '''
        ***** MAJOR FIX NEEDED HERE *****
        '''
        # convert the big extras map to a single map (a few depths rather than being 143)
        extras = np.array(extras, dtype = np.float32)
        extras = np.transpose(extras, (2, 0, 1))
        for map_ in extras:
            self.extra_base += map_

    def observe(self):
        # apply masks
        terrain_exp = self._get_masked_explored(self.status_map, self.terrain_map)
        extra_single = self._get_masked_explored(self.status_map, self.extra_base)

        # return
        return terrain_exp, extra_single, self.status_map

    def update(self, state_):
        self.status_map[:] = state_['status']
        # add rewards

