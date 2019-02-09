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
import json # convert json strings to dict
import numpy as np
from glob import glob # file handling

class DummyIOHandler():
    def __init__(self, username, server_ip, server_port):
        self.handled_conversations = 0
        self.game_done = False

        self.encoding = 'utf-8'
    
        self.path_counter = 0
        jsons, npys = [], []

        path_folder = '/Users/yashbonde/Desktop/AI/RL/freeciv-python/examples/Turn_'
        for i in range(18):
            pf = path_folder + str(i) + '/'
            jsons.extend(sorted(glob(pf + '*.json')))
            npys.extend(sorted(glob(pf + '*.npy')))
        
        self.json_paths = self._load_paths_by_types(jsons)
        self.npy_paths = self._load_maps(npys)

        self.MAX_STEP = len(self.npy_paths)
        
        self._update() # when class is called we are already ready

    def _read(self, path):
        f = open(path, encoding = self.encoding)
        string = f.read()
        f.close()
        return string

    def _read_npy(self, path):
        map_ = np.load(path).astype(np.float32)
        return map_.tolist()

    def _load_maps(self, path_npys):
        # clustering the map type by paths
        map_paths = {'extras': [], 'status': [], 'terrain': []}
        for p in path_npys:
            tok = p.split('_')
            if tok[-1] == 'extras.npy':
                map_paths['extras'].append(p)
            elif tok[-1] == 'status.npy':
                map_paths['status'].append(p)
            else:
                map_paths['terrain'].append(p)
                
        return map_paths

    def _load_paths_by_types(self, path_jsons):
        # need to cluster tha paths according to their types also 
        paths = {'actions': [], 'state': [], 'nextAction': []}
        for p in path_jsons:
            tok = p.split('_')[-1]
            if tok == 'actions.json':
                paths['actions'].append(p)
            elif tok == 'state.json':
                paths['state'].append(p)
            else:
                paths['nextAction'].append(p)

        return paths

    def _cvt_proper_json(self, state_, map_t, map_s, map_e):
        d = json.loads(state_)
        d['map']['terrain'] = map_t
        d['map']['status'] = map_s
        d['map']['extras'] = map_e
        return json.dumps(d)

    def _update(self):
        action_path = self.json_paths['actions'][self.path_counter]
        state_path = self.json_paths['state'][self.path_counter]
        map_extra_path = self.npy_paths['extras'][self.path_counter]
        map_status_path = self.npy_paths['status'][self.path_counter]
        map_terrain_path = self.npy_paths['terrain'][self.path_counter]

        # strings
        state_ = self._read(state_path)
        action_ = self._read(action_path)

        map_e = self._read_npy(map_extra_path)
        map_s = self._read_npy(map_status_path)
        map_t = self._read_npy(map_terrain_path)

        self.latest_state_string = self._cvt_proper_json(state_, map_t, map_s, map_e)
        self.latest_action_string = action_

        self.path_counter += 1
        self.handled_conversations += 1

        if self.path_counter == self.MAX_STEP:
            self.game_done = True

    def get_latest_state(self):
        return self.latest_state_string

    def get_latest_action(self):
        return self.latest_action_string

    def send_json(self, js_str):
        self._update()

    def save_state(self, path):
        self._save_json(self.latest_state_string, path)

    def save_action(self, path):
        self._save_json(self.latest_action_string, path)

    def reset(self):
        self.path_counter = 0

