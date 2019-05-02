import os
import subprocess
import fileinput
import sys


def run_nori_and_change_xml(original_line, changed_line, image_file_name, destination_folder, count):
    subprocess.run(["nori.exe", "cboxArea.xml"])
    os.rename(image_file_name, destination_folder + "\\" + str(count) + ".png")
    for line in fileinput.input("cboxArea.xml", inplace=1):
        print(line.replace(original_line, changed_line), end="")


def transform_obj(file_name, v):
    for line in fileinput.input(file_name, inplace=True):
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


# translations by z [-0.9 ,1.5]
# translations by x [-0.7,0.7]


sphere_translations = [[0,0,0],[-0.5,0,-0.5]]
trans_by_x = create_list_of_coords(-0.7, 0.7, 0.1)
trans_by_z = create_list_of_coords(-0.9, 1.5, 0.1)
list_of_trans = []
for trans in trans_by_x:
    list_of_trans.append([round(trans,1),0,0])
for trans in trans_by_z:
    if trans !=0:
        list_of_trans.append([0,0,round(trans,1)])
for trans_x in trans_by_x:
    for trans_z in trans_by_z:
        if round(trans_x,1) == 0 or round(trans_z,1) == 0:
            continue
        list_of_trans.append([round(trans_x,1), 0, round(trans_z,1)])

print("num of translations =",len(list_of_trans))
xml_file = "cboxArea.xml"
count = 0
numberOfImages = len(list_of_trans)*len(sphere_translations)
for sphere_trans in sphere_translations:
    transform_obj("meshes/sphere2.obj", sphere_trans)
    for trans in list_of_trans:
        print(count, "out of", numberOfImages)
        reverseTrans = [-trans[0],-trans[1],-trans[2]]
        print("transform light by",trans)
        transform_obj("meshes/light.obj",trans)

        original_line = "whitted"
        changed_line = "lightDepthArea"
        image_file_name = "cboxArea.png"
        destination_folder = "groundTruthArea"
        run_nori_and_change_xml(original_line, changed_line, image_file_name, destination_folder, count)

        original_line = "lightDepthArea"
        changed_line = "depthMapArea"
        image_file_name = "cboxArea.png"
        destination_folder = "lightDepthArea"
        run_nori_and_change_xml(original_line, changed_line, image_file_name, destination_folder, count)

        original_line = "depthMapArea"
        changed_line = "whittedNoShadow"
        image_file_name = "cboxArea.png"
        destination_folder = "depthMapArea"
        run_nori_and_change_xml(original_line, changed_line, image_file_name, destination_folder, count)

        original_line = "whittedNoShadow"
        changed_line = "whitted"
        image_file_name = "cboxArea.png"
        destination_folder = "noShadowsArea"
        run_nori_and_change_xml(original_line, changed_line, image_file_name, destination_folder, count)

        transform_obj("meshes/light.obj",reverseTrans)
        count = count + 1
