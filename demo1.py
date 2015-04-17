#!/usr/bin/python

'''
 demo1.py
 example of sensors reading
'''

from ipowersw import ipowersw
from time import sleep

sw = ipowersw('http://10.28.17.250/api/')

while 1:
  (state, temperature, humidity, light) = sw.get()

  s = "relay {}, ".format(("off", "on")[state])
  s += "temperature={}*C, ".format(temperature)
  s += "humidity={}%, ".format(humidity)
  s += "light={}".format(light)

  print (s)
  sleep (1)
