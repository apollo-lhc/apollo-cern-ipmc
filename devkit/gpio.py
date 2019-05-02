#!/usr/bin/env python
# encoding: utf-8
from IPMCDevLib import IPMCDevCom
from IPMCDevLib import IPMCDevMgt
from IPMCDevLib import IPMCDevATCA
from IPMCDevLib import IPMCDevAMC
from time import sleep

if __name__ == '__main__':
    IPMCDevObject = IPMCDevCom()
    
    IPMCDevATCAObject = IPMCDevATCA(IPMCDevObject)
    
    while (True):
        getio = IPMCDevATCAObject.GETIO(0x14) ## 0x14 = GPIO20
        if getio < 0:
            print("Get IO issue: {}".format(getio))
        elif getio == IPMCDevATCAObject.GND:
            print('IO : GND')
        elif getio == IPMCDevATCAObject.VCC:
            print('IO : VCC')
        sleep(0.1)

