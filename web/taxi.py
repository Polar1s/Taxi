from ctypes import *
import os

path = os.path.split(os.path.realpath(__file__))[0]

filepath = os.path.join(path, "../build/libtaxi.so");

# open libtaxi.so
libtaxi = cdll.LoadLibrary(filepath)

# configure Python to C interface
init = libtaxi.init
init.argtypes = ()
init.restype = None

position = libtaxi.position
position.argtypes = (c_double, c_double)
position.restype = c_void_p

query = libtaxi.query
query.argtypes = (c_int, c_int)
query.restype = c_void_p

release = libtaxi.release
release.argtypes = (c_void_p,)
release.restype = None
