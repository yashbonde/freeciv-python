'''
__init__.py

This is the initiator for pyfc

@yashbonde - 27.01.19
'''

from pyfc.version import VERSION as __version__
from pyfc.world import World
from pyfc.minigames import MiniGames

__all__ = ['World', 'MiniGames']