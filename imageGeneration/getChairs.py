import fileinput

def main():
	fileName = 'chair.obj'
	# sort all the strings in their corresponding lists
	textureVertices = []
	geometricVertices = []
	vertexNormals = []
	faces = []
	for line in fileinput.input(fileName)
		if line[:2] == 'vt':
			textureVertices.append(line)
		elif line[:2] == 'vn':
			vertexNormals.append(line)
		elif line[0] == 'v':
			geometricVertices.append(line)
		elif line[0] == 'f':
			faces.append(line)
		else:
			continue

	# compute the center of mass of the geometric vertices
	com = getCenterOfMass(geometricVertices)

	# arrange the vertices around the center of mass
	geometricVertices = centerObject(geometricVertices, com)

	# create rotation matrix if needed
	rotationMatrix = getRotationMatrix()

	# rotate the object
	geometricVertices = rotateObject(geometricVertices, rotationMatrix)
	# CAUTION! MIGHT NEED TO CHANGE THE VERTEX NORMALS TOO

	# get axis aligned bounding box of the object
	bbox = getAxisAlignedBoundingBox(geometricVertices)
	# bbox will have 6 elements
	# xLeft, xRight, yTop, yBottom, zNear, zFar

	# translate the object to place it on the ground in the box scene
	geometricVertices = groundObject(geometricVertices, bbox)

	# create a new obj file for the modified object
	

if __name__ == "__main__":
	main()