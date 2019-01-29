![alt_text](https://github.com/yashbonde/freeciv-python/blob/master/images/freeciv_logo_small-01.jpg)

### UNDER DEVELOPMENT `PRE-RELEASE`

![alt_text](https://img.shields.io/github/license/yashbonde/freeciv-python.svg?style=for-the-badge) ![alt_text](https://img.shields.io/github/issues/detail/s/yashbonde/freeciv-python/3.svg?style=for-the-badge)

This is the official python environment for [Freeciv 3.1](http://freeciv.org) for advancement in deep reinforcement learning. Freeciv is an open source multiplayer turn based strategy game inspired from Civilizations. The vastness of game through its state and action space makes it a formidible challenge to both humans and AI alike. Read about the idea [here](https://medium.com/@yashbonde/call-for-an-army-of-be-a-sts-f751436671be), and why it is of such importance.

I have shifted most of my work in freeciv to a seperate [repo](https://github.com/yashbonde/freeciv-related) and kept only the source files here. For some research materials go to `/research-papers` folder in the other repo. The code base is initially designed for freeciv-desktop version but it will be modular enough so we can plug it to freeciv-web version as well.

## Vision

This environment will single handedly be one of the most difficult open-sourced learning environments. I aim to make this at the forefront of deep RL research, to push the boundaries of what can be done. The end goal is to create an agent which can learn by itself (also creating the baseline model as this develops).

We also regulate the version number and count to make it most efective, currenty we are on version `0.4` corresponding with the percentage of work done. Version `1.0` is what we want make the first release version. 

## Installing The Game

You can install the game from the main [repo](https://github.com/freeciv/freeciv) by looking at the INSTALL in docs. Though this is a bit tricky on macOS and requires installation of various packages.

#### macOS

You can play the game simply by downloading the macOS package [here](https://www.dropbox.com/sh/buypyjprsbvq0hd/AABuisFfBn-WDJgAEcXIZGrSa?dl=0) and running the `.pkg` file.

## Structure of Environment

The details of the environment are given in the `/doc` folder.

## Update Logs
As the size of logs keeps increasing it has been shifted to `doc/logs`, some important milestones:

08.11.2018: Repo created with initial commit containing [Monte Carlo Agent](http://groups.csail.mit.edu/rbg/code/civ/)
