import os
import subprocess
import fileinput
import sys
from random import shuffle
import math

# randX = random.uniform(scene_min_x + d, scene_max_x - d)
# randY = random.uniform(scene_min_y + d, scene_max_y - d)
# randZ = random.uniform(scene_min_z + d, scene_max_z - d)

xml_file = "ajax.xml"

def run_nori_and_change_xml(original_line, changed_line, image_file_name, destination_folder, count):
    subprocess.run(["nori.exe", xml_file])
    os.rename(image_file_name, destination_folder + "\\" + str(count) + ".png")
    for line in fileinput.input(xml_file, inplace=1):
        print(line.replace(original_line, changed_line), end="")


def put_rand_light_pos(random_pos_str):
    for line in fileinput.input(xml_file, inplace=1):
        # in order for this to work, we assume that the xml first has empty pos value
        print(line.replace("<point name=\"position\" value=\"\"/>", random_pos_str), end="")


def put_default_light_pos(random_pos_str):
    for line in fileinput.input(xml_file, inplace=1):
        print(line.replace(random_pos_str, "<point name=\"position\" value=\"\"/>"), end="")


def transform_sphere(v):
    for line in fileinput.input("meshes/sphere2.obj", inplace=True):
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

def create_list_of_coords(min_x, max_x, step):
    current = min_x
    res_list = []
    while current <= max_x:
        res_list.append(current)
        current = current + step
    return res_list


def create_list_of_heights(original_h,step,maxSteps):
    res_list = []
    current = original_h
    while current <= original_h+step*maxSteps:
        res_list.append(current)
        current = current + step
    current = original_h
    while current >= original_h-step*maxSteps:
        current = current - step
        res_list.append(current)
    return res_list



def rotByY(angle,vec):
    return [math.cos(angle)*vec[0]+math.sin(angle)*vec[2], vec[1], -math.sin(angle)*vec[0]+math.cos(angle)*vec[2]]


def rotAjaxByY(angle):
    for line in fileinput.input("ajax.obj", inplace=True):
        if line.startswith('v '):
            newLine = line.split()
            x = float(newLine[1])
            y = float(newLine[2])
            z = float(newLine[3])
            # translate to origin
            # COM of ajax = -1.36395 17.8657 - 17.3952
            v = [1.36395, -17.8657, 17.3952]
            x = x + v[0]
            y = y + v[1]
            z = z + v[2]
            rotatedVec = rotByY(angle, [x, y, z])
            x = rotatedVec[0]
            y = rotatedVec[1]
            z = rotatedVec[2]
            # translate back to original position
            x = x - v[0]
            y = y - v[1]
            z = z - v[2]
            line = "v " + str(x) + " " + str(y) + " " + str(z) + "\n"
        sys.stdout.write(line)

def interpolatedV(light_pos, lightToCameraVec,t):
    resVec = []
    for idx in range(3):
        resVec.append((1-t)*light_pos[idx] + t*lightToCameraVec[idx])
    return resVec


scene_min_x = -0.6  # -1.02
scene_max_x = 0.6  # 1
scene_min_y = 0.2
scene_max_y = 1
scene_min_z = 2  # -1.04
scene_max_z = 5  # 0.99

d = 0.2

randXlist = create_list_of_coords(scene_min_x, scene_max_x, d)
randYlist = create_list_of_coords(scene_min_y, scene_max_y, d)
randZlist = create_list_of_coords(scene_min_z, scene_max_z, d)

shuffle(randXlist)
shuffle(randYlist)
shuffle(randZlist)

# list of vectors to incrementally transform the sphere with
# first one is 0,0,0 so we get the original position too
transformList = [[0, 0, 0], [-0.3, 0, 0], [0, 0.3, 0] , [-0.2, 0.1, 0.1], [0, -0.4, 0] , [-0.2, 0, -0.1], [-0.2, 0, 0.2]]

# randX = random.uniform(scene_min_x + d, scene_max_x - d)
# randY = random.uniform(scene_min_y + d, scene_max_y - d)
# randZ = random.uniform(scene_min_z + d, scene_max_z - d)
# 0,1.0944,3.359343
iterator = 0
# ajax COM -1.36395 17.8657 -17.3952
# -65.6055, 47.5762, 24.3583


ajax_com = [-1.36395, 17.8657, -17.3952]
vec = [-65.6055 - (-1.36395), 47.5762, 24.3583 - (-17.3952)]
scale = 4
vec = [vec[0]/scale, vec[1]/scale, vec[2]/scale]
# original y val of camera is 47.5762
camera_position = [-65.6055, 20, 24.3583]
original_light_position = [-20, 40, 20]
light_to_camera_vector = [camera_position[0] - original_light_position[0], camera_position[1] - original_light_position[1], camera_position[2] - original_light_position[2]]
'''
rotatedVec = rotByY(-math.pi/2, vec)
lightPos = [ajax_com[0]+rotatedVec[0],ajax_com[1]+rotatedVec[1],ajax_com[2]+rotatedVec[2]]
rand_pos = str(lightPos[0]) + "," + str(lightPos[1]) + "," + str(lightPos[2])
print(rand_pos)
random_pos_str = "<point name=\"position\" value=\"" + rand_pos + "\"/>"
put_rand_light_pos(random_pos_str)
subprocess.run(["nori.exe", xml_file])
'''

ajax_angles = [0, math.pi/8, math.pi/6, math.pi/4]
h_list = create_list_of_heights(47.5762, 2, 14)
heights = range(-20, 40, 4)
# h_list = [20]
print(*h_list)
count = 0
for ajax_angle in ajax_angles:
    rotAjaxByY(ajax_angle)
    t = 0
    while t <= 1:
        for h in heights:
            interpolated_vec = interpolatedV(original_light_position,light_to_camera_vector,t)
            lightPos = interpolated_vec
            print('light Pos = ',lightPos)
            rand_pos = str(lightPos[0]) + "," + str(h) + "," + str(lightPos[2])
            random_pos_str = "<point name=\"position\" value=\"" + rand_pos + "\"/>"
            print(random_pos_str)
            put_rand_light_pos(random_pos_str)

            original_line = "    <integrator type=\"simple\">"
            changed_line = "    <integrator type=\"lightDepth\">"
            image_file_name = "ajax_simple.png"
            destination_folder = "ajaxGroundTruth"
            run_nori_and_change_xml(original_line, changed_line, image_file_name, destination_folder, count)

            original_line = "    <integrator type=\"lightDepth\">"
            changed_line = "    <integrator type=\"depthMap\">"
            image_file_name = "ajax_lightDepth.png"
            destination_folder = "ajaxLightDepth"
            run_nori_and_change_xml(original_line, changed_line, image_file_name, destination_folder, count)

            original_line = "    <integrator type=\"depthMap\">"
            changed_line = "    <integrator type=\"noShadow\">"
            image_file_name = "ajax_depthMap.png"
            destination_folder = "ajaxDepthMap"
            run_nori_and_change_xml(original_line, changed_line, image_file_name, destination_folder, count)

            original_line = "    <integrator type=\"noShadow\">"
            changed_line = "    <integrator type=\"simple\">"
            image_file_name = "ajax_noShadows.png"
            destination_folder = "ajaxNoShadow"
            run_nori_and_change_xml(original_line, changed_line, image_file_name, destination_folder, count)

            put_default_light_pos(random_pos_str)
            count = count + 1
        t = t + 0.1

'''
for randZ in randZlist:
    for randY in randYlist:
        for randX in randXlist:
            rand_pos = str(randX) + "," + str(randY) + "," + str(randZ)
            print(rand_pos)
            random_pos_str = "<point name=\"position\" value=\"" + rand_pos + "\"/>"

            put_rand_light_pos(random_pos_str)

            original_line = "    <integrator type=\"simple\">"
            changed_line = "    <integrator type=\"lightDepth\">"
            image_file_name = "cbox_simple.png"
            destination_folder = "groundTruth"
            run_nori_and_change_xml(original_line, changed_line,image_file_name, destination_folder, iterator)

            original_line = "    <integrator type=\"lightDepth\">"
            changed_line = "    <integrator type=\"depthMap\">"
            image_file_name = "cbox_lightDepth.png"
            destination_folder = "lightDepth"
            run_nori_and_change_xml(original_line, changed_line,image_file_name, destination_folder, iterator)

            original_line = "    <integrator type=\"depthMap\">"
            changed_line = "    <integrator type=\"noShadow\">"
            image_file_name = "cbox_depthMap.png"
            destination_folder = "depthMap"
            run_nori_and_change_xml(original_line, changed_line, image_file_name, destination_folder, iterator)

            original_line = "    <integrator type=\"noShadow\">"
            changed_line = "    <integrator type=\"simple\">"
            image_file_name = "cbox_noShadows.png"
            destination_folder = "noShadows"
            run_nori_and_change_xml(original_line, changed_line, image_file_name, destination_folder, iterator)

            put_default_light_pos(random_pos_str)
            iterator = iterator + 1


for line in fileinput.input(xml_file, inplace=1):
    # in order for this to work, we assume that the xml first has empty pos value
    print(line.replace("simple", "depthMap"), end="")
'''


