
# pltraceback.py

"""
Traceback text formatting; just use cgitb after replacing some functions...
"""

import os.path
import time
import cgitb

time.ctime= lambda x: ''
os.path.abspath= os.path.basename
text= cgitb.text
