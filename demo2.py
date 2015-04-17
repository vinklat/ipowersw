#!/usr/bin/python

'''
 demo2.py
 power on/off depending on the ambient light intensity 
'''

from ipowersw import ipowersw
from time import sleep

sw=ipowersw('http://10.28.17.250/api/')

while 1:
  light=sw.get_light()
  print ("light intensity: {}".format(light))

  if (light < 20):
    sw.on()
  elif (light > 60):
    sw.off();

  sleep(1)
