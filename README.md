# freeciv-python

[![License: GPL v2](https://img.shields.io/badge/License-GPL%20v2-blue.svg)](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html)

This is the official python environment for [Freeciv 2.6](http://freeciv.org) for advancements in deep Reinforcement Learning. 
Freeciv is an open source multiplayer step-wise strategy game inspired from Civilizations 2. This creates a unique solution for a deep RL environment which has been long due. OpenAI gym and other environments lack this kind of games and create a void which needs to be filled for.

OpenAI gym pioneered a new way for to democratize RL for masses, and now we must aim to develop something that is even more difficult. No such environment exists as of now and this thing will push the boundary for what can be done. We are improving over the previous [work done](http://groups.csail.mit.edu/rbg/code/civ/) and writing the python3 wrapper for it.

For some research material go to `/readings` folder.

Read about the idea [here](https://medium.com/@yashbonde/call-for-an-army-of-be-a-sts-f751436671be), and why it is of such importance.

## The Game
[![ForTheBadge makes-people-smile](http://ForTheBadge.com/images/badges/makes-people-smile.svg)](http://ForTheBadge.com)

You can play the game simple by downloading the MacOS package [here](https://www.dropbox.com/sh/buypyjprsbvq0hd/AABuisFfBn-WDJgAEcXIZGrSa?dl=0) and running the `.pkg` file. It is fully interactive and a really fun game to play, though idea is that we should be able to compile and run it.

![alt text](https://vignette.wikia.nocookie.net/freeciv/images/1/1c/Freeciv-growing-cities-steal-food.jpg/revision/latest?cb=20150409102837)

## Structure of Environment
The structure of package is inspired from VizDoom and the ease of use is inspired from OpenAI gym. More details will be added here. Initial commit contains the package for running Monte Carlo agent written in C++.

## Vision
This game environment aims to be at the forefront of deep RL research, to push the boundaries of what can be done. The end goal is to create an agent which can learn by itself with minimal help from humans (the only support should be basic instructions). 

## Credits
The credit for the base code has to be given to the authors of [freeciv repo](https://github.com/freeciv/freeciv) and the authors of [paper](http://groups.csail.mit.edu/rbg/code/civ/).

You can reach me out at bonde.yash97@gmail.com
