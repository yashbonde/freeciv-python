'''
setup.py

@yashbonde 29.01.2019
'''

import setuptools

with open("README.md", "r") as fh:
    long_description = fh.read()

setuptools.setup(
    name="freeciv-python",
    version="0.0.4",
    author="Yash Bonde",
    author_email="bonde.yash97@gmail.com",
    description="Worlds most advanced learning environment",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/yashbonde/freeciv-python",
    packages=setuptools.find_packages(),
    classifiers=[
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: GNU GPLv2",
        "Operating System :: OS Independent",
    ],
)