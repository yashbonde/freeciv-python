![alt_text](https://github.com/yashbonde/freeciv-python/blob/master/images/freeciv_logo_small-01.jpg)

### UNDER DEVELOPMENT `PRE-RELEASE`

![alt_text](https://img.shields.io/github/license/yashbonde/freeciv-python.svg?style=for-the-badge) ![alt_text](https://img.shields.io/github/issues/detail/s/yashbonde/freeciv-python/3.svg?style=for-the-badge)

This is the official python environment for [Freeciv 3.1](http://freeciv.org) for advancements in deep Reinforcement Learning. Freeciv is an open source multiplayer step-wise strategy game inspired from Civilizations 2. This creates a unique solution for a deep RL environment which has been long due. OpenAI gym and other environments lack this kind of games and create a void which needs to be filled for.

OpenAI gym pioneered a new way for to democratize RL for masses, and now we must aim to develop something that is even more difficult. No such environment exists as of now and this thing will push the boundary for what can be done.

Read about the idea [here](https://medium.com/@yashbonde/call-for-an-army-of-be-a-sts-f751436671be), and why it is of such importance.

For some research material go to `/readings` folder. This will be removed and shifted to some folder as we approach the release for this project.

## The Game

You can play the game simple by downloading the MacOS package [here](https://www.dropbox.com/sh/buypyjprsbvq0hd/AABuisFfBn-WDJgAEcXIZGrSa?dl=0) and running the `.pkg` file. It is fully interactive and a really fun game to play, though idea is that we should be able to compile and run it.

![alt text](https://vignette.wikia.nocookie.net/freeciv/images/1/1c/Freeciv-growing-cities-steal-food.jpg/revision/latest?cb=20150409102837)

You can also install the game from the main [repo](https://github.com/freeciv/freeciv) by looking at the INSTALL in docs.

## Vision
This game environment aims to be at the forefront of deep RL research, to push the boundaries of what can be done. The end goal is to create an agent which can learn by itself with minimal help from humans (the only support should be basic instructions). For more details kindly go through `README`.

## Installation
Game not developed but installation should be as easy as:

```sh
$ pip3 install freeciv
```

And just like any open-source project worth it's salt, proper documentation is available in the `/doc` folder.

## Structure of Environment
The structure of package is inspired from VizDoom and the ease of use is inspired from OpenAI gym. The freeciv-python has a three part structure, the first part is the `packet_manager` which handles the IO, secondly `fc_inference` which converts the information from packets to structure understandable by deep learning models. Lastly we have the `world` which is the env where the game is played out.

## Understanding the Game
As this project comes to realisation moving all the side work done in this repo to a new [freeciv-related](https://github.com/yashbonde/freeciv-related) repo.

## Update Logs
DD-MM-YYYY format used for dates

18.01.2019: Make new repo [freeciv-related](https://github.com/yashbonde/freeciv-related) to shift the side work done on freeciv.

10.01.2019: Add final structure and corresponding folders.

09.12.2018: Finalise structure as client socket conection started adding template files in `/freeciv` folder. The structure is inspired by [freeciv-bot](https://github.com/chris1869/freeciv-bot) and builds on top of it. Remove MonteCarlo agent, complete refractoring.

19.11.2018: Add template for freeciv-python environment, find templates and further ideas in `/python_core` folder

08.11.2018: Repo created with initial commit containing [Monte Carlo Agent](http://groups.csail.mit.edu/rbg/code/civ/)

## Credits
The credit for the base code has to be given to the authors of [freeciv repo](https://github.com/freeciv/freeciv).
