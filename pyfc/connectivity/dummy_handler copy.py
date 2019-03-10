'''
main_io.py

This is the main IO handler file which will perform IO loops with the server

@yashbonde - 28.02.2019
'''

#dependencies
import json # convert json strings to dict
import numpy as np
from glob import glob # file handling

class IOHandler():
    def __init__(self, username, server_ip, server_port):
        self.username = username
        self.server_ip = server_ip
        self.server_port = server_port
        
        self._update() # when class is called we are already ready

    def _save_json(self, str, path):
        with open(path, 'w', encoding = utf-8) as f:
            f.write(str)

    def _read(self, path):
        '''
        read what is present the the io_buffer and send the JSON back to FCIOManager
        '''
        f = self._Buffer.latest_text
        return f

    def _update(self):
        '''
        backend update function to handle the crude details
        '''
        pass

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
        pass

