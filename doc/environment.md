# Freeciv Environment

## What is freeciv?

Freeciv is an open source turn based strategy game similar to Civilizations. 

## Observations

Freeciv is a natural extension of board games, and so we call the state of game at any given instance as "board state". Each board state is a sum of various independent states like topology, ownership, etc. which are represented using different "maps" or "feature layers". Each board state is divided into two types:

1. **Time Invariant**: This board state is fixed for each player throughout the game and does not change with time. For e.g. `topology_map` is fixed throughout the game, though different rulesets might allow for different things.

2. **Time Variant**: This board state changes with time, e.g. `ownership_map` tells about the ownership of tiles. This will change as time progresses, some countries will get big while some will vanish.

3. **Player Perspective**: One thing that is different for each player is how it will look at the world. Different countries would have explored different areas and so each country will have a unique map of their own. This is implemented using masking. Using masks a single map can be divided for each player increasing the code efficiency.

The board os of shape `(M, N)` where `M` and `N` is the size of the board.

### Map Descriptions

Descriptions of various maps is given below:

**Time Invariant Maps:**

1. `topology_map`: implemented [here]() tells about the topology of the board. The symbols and meanings are given below:

```
Glacier    a
Deep Ocean :
Desert     d
Forest     f
Plains     p
Grasslands g
Hills      h
Jungle     j
Lake       +
Mountains  m
Ocean   <Space>
Swamp      s
Tundra     t
```

**Time Variant Maps:**

2. `ownership_map`: implemented [here]() tells about the onwership of tiles on the board

3. `resources_map`: implemented [here]() tells about the various resources available on the tiles. The resources are given as follows:

```
buffalo   b
coal      c
fish      y
fruit     f
furs      u
game      e
gems      g
gold      $
iron      /
ivory     i
oasis     o
oil       x
peat      a
pheasant  p
resources r
silk      s
spice     t
whale     v
wheat     j
wine      w
```

4. `units_map`: implemented [here]() tells about the position of various units across the board

## Actions

For now we directly convert the action space of units with what is given in the C code. The list of all actions for a particular unit can be obtained with function `get_actions()` which gives a list of all the possible actions for that unit whereas `get_valid_actions()` returns a list of valid actions at that state.

```python
for unit in unit_list:
    print(f"[*] All actions for unit {unit.id} are:")
    print(unit.get_actions())

    print(f"[*] Valid actions for unit {unit.id} in given state are:")
    print(unit.get_valid_actions())
```
