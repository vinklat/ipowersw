#!/usr/bin/python

'''
  simple python class for ipowersw (ip relay) control
  using REST API
'''

import requests
import json

class ipowersw:
  url=""

  def on(self):
    r = requests.post(self.url + "1")
    data=json.loads(r.text)
    return (bool(data['relay']))
    
  def off(self):
    r = requests.post(self.url + "0")
    data=json.loads(r.text)
    return (bool(data['relay']))

  def get(self):
    r = requests.get(self.url)
    data = json.loads(r.text)
    return ( bool(data['relay']), float(data['temperature']), float(data['humidity']), int(data['light']) )

  def get_state(self):
    return get(self)[0]

  def get_temp(self):
    return get(self)[1]

  def get_hum(self):
    return get(self)[2]

  def get_light(self):
    return self.get()[3]

  def __init__(self, url):
    self.url=url

