import os
import subprocess
import fileinput
import sys
from random import shuffle

# randX = random.uniform(scene_min_x + d, scene_max_x - d)
# randY = random.uniform(scene_min_y + d, scene_max_y - d)
# randZ = random.uniform(scene_min_z + d, scene_max_z - d)


def run_nori_and_change_xml(original_line, changed_line, image_file_name, destination_folder, count):
    subprocess.run(["nori.exe", "cbox.xml"])
    os.rename(image_file_name, destination_folder + "\\" + str(count) + ".png")
    for line in fileinput.input("cbox.xml", inplace=1):
        print(line.replace(original_line, changed_line), end="")


def put_rand_light_pos(random_pos_str):
    for line in fileinput.input("cbox.xml", inplace=1):
        # in order for this to work, we assume that the xml first has empty pos value
        print(line.replace("<point name=\"position\" value=\"\"/>", random_pos_str), end="")


def put_default_light_pos(random_pos_str):
    for line in fileinput.input("cbox.xml", inplace=1):
        # in order for this to work, we assume that the xml first has empty pos value
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

print(*randXlist)
shuffle(randXlist)
print(*randXlist)
shuffle(randYlist)
shuffle(randZlist)

# list of vectors to incrementally transform the sphere with
# first one is 0,0,0 so we get the original position too
transformList = [[0, 0, 0], [-0.3, 0, 0], [0, 0.3, 0] , [-0.2, 0.1, 0.1], [0, -0.4, 0] , [-0.2, 0, -0.1], [-0.2, 0, 0.2]]

# transform_sphere([0, 0.5, 0.2])
print(randYlist)


# randX = random.uniform(scene_min_x + d, scene_max_x - d)
# randY = random.uniform(scene_min_y + d, scene_max_y - d)
# randZ = random.uniform(scene_min_z + d, scene_max_z - d)
# 0,1.0944,3.359343
iterator = 0

for v in transformList:
    transform_sphere(v)  # transform sphere (incrementally) and render it will all different light positions
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


for line in fileinput.input("cbox.xml", inplace=1):
    # in order for this to work, we assume that the xml first has empty pos value
    print(line.replace("simple", "depthMap"), end="")



