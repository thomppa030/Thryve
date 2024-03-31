#!/bin/bash

#Creates the Python environment
python3 -m venv venv

#Activates the virtual environment
source venv/bin/activate

#install necessary packages
pip install -r requirements.txt

#deactivate Python environment
deactivate
