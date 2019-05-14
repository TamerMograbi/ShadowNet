import os
import shutil

def main():
	# run this program from the folder which will have objFiles folder
	if os.path.exists('objFiles'):
		shutil.rmtree('objFiles')

	counter = 0
	os.mkdir("objFiles")
	# pathName is the folder which has all the object files
	pathName = 'C:\\Users\\crica\\Downloads\\IKEA_models\\IKEA'
	for root, dirs, files in os.walk(pathName):
		for file in files:
			fileName = os.path.join(root, file)
			if 'table' in fileName or 'chair' in fileName or 'desk' in fileName:
				name = fileName[fileName.rfind('\\') + 1:]
				if '.obj' in name:
					name = str(counter).zfill(5) + '.obj'
					shutil.copy(fileName, os.path.join('C:\\Users\\crica\\Documents\\UMD\\CMSC740AdvancedComputerGraphics\\pytorch-CycleGAN-and-pix2pix\\imageGeneration\\objFiles', name))
					counter += 1

if __name__ == '__main__':
	main()