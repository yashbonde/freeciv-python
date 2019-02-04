# Freeciv MiniGames

MiniGames are the most important aspect of `pyfc` module, it not only increases accessibility but also
makes it easier for agents to train on the game. Learning smaller things before it moves onto learn bigger
and more complex things like playing the complete game.

![alt-text](http://4.bp.blogspot.com/-F_jksKfS_-s/U2_XQ4X6yOI/AAAAAAAAIKM/hT4TYa1xm3U/s1600/FreeCiv.png)

This is a list of MiniGames that we intned to develop, they are classified according to the increasing levels of
difficulty.

    NOTE: THIS IS INITIAL DISCUSSION, NOT FINAL PRODUCT.

## MapExplorer

This is a set of challenges where the aim of the agent to explore map as much as possible while maximising the resources
collected. This is similar to `CollectMineralShards` from `pysc2`. The main task is to move around on the map and
collect resources, which may be setup in form of villages. The aim of this is to train the agent how to move around
in an environment. The player starts with few competitors on a considerably large map to move around and collect
resources on the map.

<!-- Deep Learning: Use same neural network for different players, this way it has larger diverse dataset learns quicker -->

Like everything else in `pyfc` custom reward functions can be used, though we provide default rewards functions too.

## DefeatOpponents

This is a set of challenges focused more on just combat where the goal of the agent will be to kill all the opponent units.
This challenge may vary from 1 enemy to multi enemy where each plyer is trying to kill the other player. The aim of this
is to train the agent to learn to combat with other units.

<!-- Deep Learning: Use same neural network for different players, this way it has larger diverse dataset learns quicker -->


## BulbsResearch

Unlike the previous challenges this involves agent trying to achieve a fix number of research bulbs in minimum number of steps. This way the
agent has to use different elements of the game like `technology`, `government`, `cities` and `units` to finish this
task efficiently.

## TechResearch

Similar to the BulbResearch test this one will also involve agent trying to flex all its muscles to play the game efficiently
to reach the desired goal. `technology` is a bit tricky becuase in order to research a particular technology, it is
important for the agent to have a minimum number of `bulbs`.


