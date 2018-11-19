# Sample Package Templates

Each file here is a richly documented temaplate to make ease of use and development a priority, in that particular order. We aim to develop a package which can be used by adavenced user as well as layman with equal use.

## Why use Rich Template Style?

I believe that rather than writing a HOWTO file we should document everthing as if we are the ones who are going to use it. This though will create huge files filled with 90% comments reduces the difficulty barrier for the simple user. This way is better for both the user and developer to see what is doing what. This is the best approach we have till better visualisation mechanisms are built.

## Stucture

The structure of freeciv package is relatively simple, the freeciv folder has the templates for the backend functions.

1. `template.py`: the main template file has user end template. This shows how different functions that the user has are interacts with the backend and the steps to follow run the package.

Under freeciv folder:

2. `world.py`: file for environment. It has following functions