import sys
import os
os.sys.path.append('/usr/local/lib/python/site-packages')
os.sys.path.append('/usr/ali/lib/python2.5/site-packages')
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))
from raweb.main import *
application = make_app()