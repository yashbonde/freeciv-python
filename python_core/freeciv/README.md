## Backend

The description of the files is here:

1. `world.py`: This is the main package file that is called and contains the functionality for running the environment. It has been designed to have only the minimum required methods to get it up and running. Where possible functions have been pushed to other files.

2. `core_wrapper.py`: It has the functions connecting to the backend and is used to send server commands and return the information in rquired format. For eg. the maps are to be returned as large numpy arrays, so it handles the data conversion as well.

3. `map_utils.py`: Has information and basic functions related to conversion of maps.

4. `unit.py`: all the functionality of the unit converted into a class

5. `utils.py`: util functions

6. `learner_comms.py`: server commands

### Under `/src`

1. `CppFreecivWrapper.h/cpp`: This is the C++ part for `core_wrapper.py`, it has the class to run the ops

2. `Interface.h/cpp`: functions to connect to the server using Socket connections

3. `learner_comms.h`: server commands 