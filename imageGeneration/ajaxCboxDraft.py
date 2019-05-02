import os
import subprocess
import fileinput
import sys
from random import shuffle
import math

def transform_ajax(v):
    for line in fileinput.input("ajax.obj", inplace=True):
        if line.startswith('v '):
            newLine = line.split()
            x = float(newLine[1])
            y = float(newLine[2])
            z = float(newLine[3])
            x = x+v[0]
            y = y+v[1]
            z = z+v[2]
            line = "v " + str(x) + " " + str(y) + " " + str(z) + "\n"
        sys.stdout.write(line)

xml_file = "cboxArea.xml"
# ajax center of mass = <-1.36395 17.8657 -17.3952>
#in this transform i just took a random point on the sphere from sphere2.object
# and a random point from ajax and created the vector from the subtraction.
# this way ajax should have showed up in the position of the sphere (and i wanted to work from there)
# i really didn't try a lot of things here. i decided that it's better to focus on sphere first + area light first
transform_ajax([-6.23+(0.44+6.32),27.31+(0.36-27.31),-21.52+(0.051+21.52)])
subprocess.run(["nori.exe", xml_file])