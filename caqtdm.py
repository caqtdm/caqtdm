import ctypes
import threading
import time
import os

from ctypes import *

def thread1(name):
	print("Thread starting, load caQtDM_Lib with file", name)
	dll9.restype = c_int
	dll9.argtypes = [c_char_p]
	dll9.caQtDM_Create(b"python.ui")

def thread2(name):
	print("Thread starting", name)
	value = c_double();
	while True:
		time.sleep(2)
		pv= ctypes.create_string_buffer(100)
		dll9.getDataValue(b"cameter", pv, ctypes.sizeof(pv), byref(value))
		print(pv.value, value);
		dll9.getDataValue(b"catogglebutton", pv, ctypes.sizeof(pv), byref(value))
		print(pv.value, value);
		if value.value == 1.0: dll9.setDataValue(b"catogglebutton", c_double(0), b" ")
		dll9.getDataValue(b"pushbutton", pv, ctypes.sizeof(pv), byref(value))
		print(pv.value, value);
		if value.value == 1.0: dll9.setDataValue(b"pushbutton", c_double(0), b" ")
		dll9.getDataValue(b"checkbox", pv, ctypes.sizeof(pv), byref(value))
		print(pv.value, value);
		if value.value == 1.0: dll9.setDataValue(b"checkbox", c_double(0), b" ")

def thread3(name):
	print("Thread starting", name)
	value = 1.0
	while True:
		time.sleep(2)
		value = value + 1.0
		dll9.setDataValue(b"testvalue1", c_double(value), b"muA")
		dll9.setDataValue(b"testvalue2", c_double(value), b"muA")
		dll9.setDataValue(b"testvalue3", c_double(value), b"muA")
		dll9.setDataValue(b"testvalue4", c_double(value), b"muA")
		dll9.setDataValue(b"testvalue5", c_double(value), b"muA")
		dll9.setDataValue(b"testvalue6", c_double(value), b"muA")

def main():
	print('Please wait while the program is loading...')
	path = os.getenv("QT_PLUGIN_PATH")
	print(path)

	dll1 = ctypes.CDLL(path + "/libedlParser.so", mode=ctypes.RTLD_GLOBAL)
	dll2 = ctypes.CDLL(path + "/libadlParser.so", mode=ctypes.RTLD_GLOBAL)
	dll3 = ctypes.CDLL(path + "/libqtcontrols.so", mode=ctypes.RTLD_GLOBAL)
	dll4 = ctypes.CDLL(path + "/controlsystems/libepics3_plugin.so", mode=ctypes.RTLD_GLOBAL)
	dll5 = ctypes.CDLL(path + "/controlsystems/libepics4_plugin.so", mode=ctypes.RTLD_GLOBAL)
	dll6 = ctypes.CDLL(path + "/controlsystems/libdemo_plugin.so", mode=ctypes.RTLD_GLOBAL)
	dll7 = ctypes.CDLL(path + "/controlsystems/libarchiveSF_plugin.so", mode=ctypes.RTLD_GLOBAL)
	dll8 = ctypes.CDLL(path + "/controlsystems/libenvironment_plugin.so", mode=ctypes.RTLD_GLOBAL)
	global dll9 
	dll9 = ctypes.CDLL("libcaQtDM_Lib.so")
	x = threading.Thread(target=thread1, args=(1,))
	x.start()
	y = threading.Thread(target=thread2, args=(2,))
	y.start()

	z = threading.Thread(target=thread3, args=(3,))
	z.start()

if __name__ == '__main__':
    main()




