import numpy as np
import subprocess
import fileinput
import argparse
import random
import math
import os

renderedImageCounter = 0

def createNumpyMatrix(geometricVertices):
    """Parse the strings from the obj file and convert them into numpy matrix of floats to perform math efficiently"""
    vertices = []
    for line in geometricVertices:
        # convert the string to floats for x,y,z coordinates
        elements = list(map(lambda x: float(x), line.split()[1:]))
        vertices.append(elements)

    # convert to 3 x numPoints matrix
    vertices = np.asarray(vertices)
    vertices = vertices.T
    #print(vertices.shape)

    return vertices

def getCenterOfMass(geometricVertices):
    # com will be a 3x1 vector
    com = np.average(geometricVertices, axis=1)
    com = com.reshape(3,1)

    return com

def centerAndScaleObject(geometricVertices, com):
    """Translate the object vertices so that they are centered around the origin"""
    geometricVertices = geometricVertices - com

    stdev = np.std(geometricVertices, axis=1) / 2.0
    stdev = stdev.reshape(3,1)
    geometricVertices = geometricVertices / stdev

    return geometricVertices

def getRotationMatrix():
    angleX = round(random.uniform(0, 2*math.pi), 2)
    angleY = round(random.uniform(0, 2*math.pi), 2)
    angleZ = round(random.uniform(0, 2*math.pi), 2)

    Rx = np.array([[1, 0, 0], [0, math.cos(angleX), -math.sin(angleX)], [0, math.sin(angleX), math.cos(angleX)]])
    Ry = np.array([[math.cos(angleX), 0, math.sin(angleX)], [0, 1, 0], [-math.sin(angleX), 0, math.cos(angleX)]])
    Rz = np.array([[math.cos(angleX), -math.sin(angleX), 0], [math.sin(angleX), math.cos(angleX), 0], [0, 0, 1]])

    R = np.matmul(np.matmul(Rx, Ry), Rz)
    #R = np.identity(3)
    return R

def rotateObject(geometricVertices, rotationMatrix):
    """Perform matrix multiplication - Rx to get the vertex coordinates after rotation"""
    rotatedGeometricVertices = np.matmul(rotationMatrix, geometricVertices)

    return rotatedGeometricVertices

def getAxisAlignedBoundingBox(geometricVertices):
    mins = np.amin(geometricVertices, axis=1)
    maxs = np.amax(geometricVertices, axis=1)

    # bbox will have 6 elements
    # xLeft, xRight, yTop, yBottom, zNear, zFar
    bbox = {'xmin':mins[0], 'xmax':maxs[0], 'ymin':mins[1], 'ymax':maxs[1], 'zmin':mins[2], 'zmax':maxs[2]}
    #print("bounding box:", bbox)

    return bbox

def positionObjectInTheBox(geometricVertices, bbox, com):
    """Calculate the bounds of the places where the object can be placed inside the box scene and place the object there"""
    # assumption - the object can fit inside the box scene entirely
    # the scaling to have 5 units standard deviation is just for that

    # create the range tuple in which xCom of object can lie - (min, max)
    xComRange = (-10.0 - bbox['xmin'], 10.0 - bbox['xmax'])
    yComRange = (-10.0 - bbox['ymin'], 10.0 - bbox['ymax'])
    zComRange = (20.0 + bbox['zmin'], 30.0 - bbox['zmax'])

    # generate the position - (x,y,z) for the com of the object within the above computed range
    # assume uniform distribution
    x = round(random.uniform(xComRange[0], xComRange[1]), 2)
    y = round(random.uniform(yComRange[0], yComRange[1]), 2)
    z = round(random.uniform(zComRange[0], zComRange[1]), 2)

    # translate the object so that it is now located at the above randomly generated location
    newCom = np.array([x,y,z]).reshape(3,1)
    #newCom = np.array([0,-5,25]).reshape(3,1)
    geometricVertices = geometricVertices + newCom

    return geometricVertices

def npMatrixToStrings(geometricVertices):
    stringList = []
    for vertex in geometricVertices.T:
        line = "v " + str(vertex[0]) + " " + str(vertex[1]) + " " + str(vertex[2]) + "\n"
        stringList.append(line)

    return stringList

def removeTextureVertices(faces):
    newFaces = []
    for line in faces:
        elements = line.split()[1:]
        # elements = ['f', 'v/vt/vn', 'v/vt/vn', 'v/vt/vn']
        # elements = ['f', '1231/14134/2341', '12/24/432', '342/345/67']
        # we want following
        # elements = ['f', '1231/14134/2341', '12/24/432', '342/345/67']
        for index, face in enumerate(elements):
            startIndex = face.find('/')
            endIndex = face.rfind('/')+1
            toReplace = face[startIndex:endIndex]
            face = face.replace(toReplace, "//")

            elements[index] = face

        newLine = 'f ' + elements[0] + " " + elements[1] + " " + elements[2] + "\n"
        newFaces.append(newLine)

    return newFaces

def printFirstThreeVertices(geometricVertices):
    print(len(geometricVertices))
    for i in range(6):
        print(geometricVertices.T[i])
        #print(geometricVertices[i])

def renderImages(lightType):
    if lightType == 'point':
        subprocess.run(["nori.exe", "custom_simple.xml"])
    else:
        subprocess.run(["nori.exe", "custom_whitted.xml"])

    subprocess.run(["nori.exe", "custom_light.xml"])

    subprocess.run(["nori.exe", "custom_depth.xml"])

    subprocess.run(["nori.exe", "custom_noShadow.xml"])


def randomizeObject(meshFile):
    fileName = meshFile
    objFile = open(fileName, 'r')
    # sort all the strings in their corresponding lists
    textureVertices = []
    geometricVertices = []
    vertexNormals = []
    faces = []
    for line in objFile:
        if line[:2] == 'vt':
            # texture vertices
            textureVertices.append(line)
        elif line[:2] == 'vn':
            # vertex normals
            vertexNormals.append(line)
        elif line[0] == 'v':
            # geometricVertices
            geometricVertices.append(line)
        elif line[0] == 'f':
            # faces
            faces.append(line)
        else:
            continue
    objFile.close()

    # create numpy matrix from the vertices string
    geometricVertices = createNumpyMatrix(geometricVertices)

    # compute the center of mass of the geometric vertices matrix
    com = getCenterOfMass(geometricVertices)

    # arrange the vertices around the center of mass
    # scale the object so that its vertices have 5 units standard deviation from the mean
    geometricVertices = centerAndScaleObject(geometricVertices, com)

    # ROTATION SHOULD HAPPEN AFTER CENTERING AND SCALING THE OBJECT AND BEFORE TRANSLATING IT
    # TO ITS NEW POSITION, IT BECOMES EASIER THAT WAY
    # create rotation matrix if needed
    rotationMatrix = getRotationMatrix()

    # rotate the object
    geometricVertices = rotateObject(geometricVertices, rotationMatrix)
    # CAUTION! MIGHT NEED TO CHANGE THE VERTEX NORMALS TOO

    # get axis aligned bounding box of the object
    bbox = getAxisAlignedBoundingBox(geometricVertices)
    # bbox will have 6 elements
    # xLeft, xRight, yTop, yBottom, zNear, zFar

    # translate the object to position it SOMEWHERE in the box scene
    geometricVertices = positionObjectInTheBox(geometricVertices, bbox, com)

    # convert the modified geometricVertices back to strings
    geometricVertices = npMatrixToStrings(geometricVertices)

    # remove texture vertices information from faces list
    #faces = removeTextureVertices(faces)

    # create a temporary obj file for the modified object
    fileName = 'tempMesh.obj'
    objFile = open(fileName, 'w')
    # write the geometric vertices to file first up
    for line in geometricVertices:
        objFile.write(line)
    # next fill up the texture vertices
    objFile.write("\n")
    for line in textureVertices:
        objFile.write(line)
    # next fill up the vertex normals
    objFile.write("\n")
    for line in vertexNormals:
        objFile.write(line)
    # next fill up the faces
    objFile.write("\n")
    for line in faces:
        objFile.write(line)

    objFile.close()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(prog="Data generator")
    parser.add_argument('-mesh', '--mesh_folder_path', required=True, help="Path to meshes folder")
    parser.add_argument('-light', '--lightType', required=True, help="Light type: 'point' or 'area'")
    parser.add_argument('-dst', '--dst_folder_path', required=True, help="Folder name where aligned images are to be saved")
    
    args = parser.parse_args()    
    meshFolder = args.mesh_folder_path
    lightType = args.lightType
    dstFolder = args.dst_folder_path

    meshFiles = [os.path.join(meshFolder, f) for f in os.listdir(meshFolder) if os.path.isfile(os.path.join(meshFolder, f))]

    for meshFile in meshFiles:
        for i in range(10):
            # create images for 10 random poses of this mesh
            randomizeObject(meshFile)
            #randomizeLight(lightType)
            # render the images now!
            renderImages(lightType)
            #alignImages(dstFolder)