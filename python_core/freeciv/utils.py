'''
Mics utils file required to run the freeciv enviroment
'''
# importing the dependencies
import numpy as np
from ast import literal_eval

# TODO: add functionality for comments in the config file
def parse_config(config_path):
    # load config file, parse it to dict and return dict
    file = open(config_path)
    all_text = file.read()
    all_text = all_text.replace(' ', '')  # remove spaces in text
    data = all_text.split(';')

    # make the dict
    config_dict = {}
    for d in data:
        key, val = d.split('=')
        config[key] = literal_eval(val)

    # return
    return config_dict