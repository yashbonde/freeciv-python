'''
dummy_handler.py

This is a very crucial file as this partially completes the projects and ables
us to train deep learning models. We do this by doing the following,
    1. this is the file that fciomanager calls, to hadle the packet delivery
    2. it ignores the packet and returns the next state that we have in game samples
    3. convert the numpy maps to dict and add those to JSON loaded dict
    4. convert back to JSON string and return

@yashbonde - 01.02.2019
'''

#dependencies
import json
import numpy as np
from glob import glob # file handling

class DummyIOHandler():
    def __init__(self):
        self.handled_conversations = 0


        self.json_being_handled = None
        self.npy_being_handled = None

        path_folder = '../../examples/'
        jsons = glob(path_folder + '*.json')
        maps = glob(path_folder + '*.npy')

        
        self.maps_clustered = self.load_paths(maps)
        self.states_clustered = self.load_paths(jsons)

    def load_paths(self, inp_paths):
        # clustering the files based on turns
        turn_clus_files = [[] for _ in range(18)]
        for f in inp_paths:
            turn_num = int(f.split('_')[1][-2:])
            turn_clus_files[turn_num].append(f)

        for i in range(len(turn_clus_files)):
            turn_clus_files[i] = sorted(turn_clus_files[i])

        return turn_clus_files


    def send_JSON(self, inp_):
        self.logger.info()
        pass

    def get_JSON(self):
        string = self._make_JSON()

    def _load_npy(self):
        # load the numpy arrays
        holding_map = np.load(self.holding_path)
        pass

    def _make_JSON(self):
        pass