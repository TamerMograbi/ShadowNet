import os.path
from data.base_dataset import BaseDataset, get_params, get_transform
from data.image_folder import make_dataset
from PIL import Image
import cv2
import numpy as np

class AlignedDataset(BaseDataset):
    """A dataset class for paired image dataset.

    It assumes that the directory '/path/to/data/train' contains image pairs in the form of {A,B}.
    During test time, you need to prepare a directory '/path/to/data/test'.
    """

    def __init__(self, opt):
        """Initialize this dataset class.

        Parameters:
            opt (Option class) -- stores all the experiment flags; needs to be a subclass of BaseOptions
        """
        BaseDataset.__init__(self, opt)
        self.dir_AB = os.path.join(opt.dataroot, opt.phase)  # get the image directory
        self.AB_paths = sorted(make_dataset(self.dir_AB, opt.max_dataset_size))  # get image paths
        assert(self.opt.load_size >= self.opt.crop_size)   # crop_size should be smaller than the size of loaded image
        self.input_nc = self.opt.output_nc if self.opt.direction == 'BtoA' else self.opt.input_nc
        self.output_nc = self.opt.input_nc if self.opt.direction == 'BtoA' else self.opt.output_nc
    """
    def __getitem__(self, index):
        Return a data point and its metadata information.

        Parameters:
            index - - a random integer for data indexing

        Returns a dictionary that contains A, B, A_paths and B_paths
            A (tensor) - - an image in the input domain
            B (tensor) - - its corresponding image in the target domain
            A_paths (str) - - image paths
            B_paths (str) - - image paths (same as A_paths)
        
        # read a image given a random integer index
        AB_path = self.AB_paths[index]
        AB = Image.open(AB_path).convert('RGB')
        # split AB image into A and B
        w, h = AB.size
        w2 = int(w / 2)
        A = AB.crop((0, 0, w2, h))
        B = AB.crop((w2, 0, w, h))

        print("before: ", A.size)

        # apply the same transform to both A and B
        transform_params = get_params(self.opt, A.size)
        A_transform = get_transform(self.opt, transform_params, grayscale=(self.input_nc == 1))
        B_transform = get_transform(self.opt, transform_params, grayscale=(self.output_nc == 1))

        A = A_transform(A)
        B = B_transform(B)

        print("after: ", A.shape)

        return {'A': A, 'B': B, 'A_paths': AB_path, 'B_paths': AB_path}
    """
    def __getitem__(self, index):
        """Return a data point and its metadata information.

        Parameters:
            index - - a random integer for data indexing

        Returns a dictionary that contains A, B, A_paths and B_paths
            A (tensor) - - an image in the input domain
            B (tensor) - - its corresponding image in the target domain
            A_paths (str) - - image paths
            B_paths (str) - - image paths (same as A_paths)
        """
        # read a image given a random integer index
        AB_path = self.AB_paths[index]
        AB = cv2.imread(AB_path, cv2.IMREAD_COLOR)
        # split AB image into A {noShadow, lightMap, depthMap} and B {groundTruth}
        height = AB.shape[0]
        width = AB.shape[1]
        #print("AB shape: ", AB.shape)

        noShadow = AB[:, :int(width/4), :]
        lightMap = AB[:, int(width/4):int(width/2), :]
        depthMap = AB[:, int(width/2):3 * int(width/4), :]

        A = np.stack((noShadow[:,:,0], noShadow[:,:,1], noShadow[:,:,2], \
                      lightMap[:,:,0], lightMap[:,:,1], lightMap[:,:,2], \
                      depthMap[:,:,0]), axis=2)
        # A has 7 channels - 3, 3, 1

        #print("A before: ", A.shape)
        # B has 3 channels
        B = AB[:, 3*int(width/4):width, :]

        A_transform = get_transform(self.opt, None, A.shape[2]) # third argument is number of channels
        B_transform = get_transform(self.opt, None, B.shape[2]) # third argument is number of channels

        #A_transform = get_transform(self.opt, None, grayscale=(self.input_nc == 1))  # third argument is number of channels
        #B_transform = get_transform(self.opt, None, grayscale=(self.input_nc == 1))  # third argument is number of channels

        A = A_transform(A)
        B = B_transform(B)
        #print("A after: ", A.shape)
        return {'A':A, 'B':B, 'A_paths': AB_path, 'B_paths': AB_path}

    def __len__(self):
        """Return the total number of images in the dataset."""
        return len(self.AB_paths)
