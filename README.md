# Shadow Net
Rendering a scene with global illumination can be a time consuming task especially when we have complex objects and multiple light sources in the scene. Reducing this render time is our motivation behind applying deep learning methods for adding shadows to a scene. Given a rendered image of the scene without shadows, and a certain representation of the type and location of the light source, we aim to apply the image to image transformation paradigm to generate an image with shadows added. This problem entails prediction of occlusion of objects in the scene and also understanding the structure and orientation of the objects.

# Data Generation
We used the Nori renderer to generate images for training, validation and testing. This saved us the time and efforts to learn using some other renderer and allowed us to spend more time on the main image to image transformation part. We had to make some additions to the Nori renderer as per our use cases which have been described below. The network takes an image as input, so we had to convert all our inputs to images and arrange them in the required format. Based on the experiments we ran, we had two types of inputs:
    
* 4 image grid (cases 1 to 5) has the image without shadows, the light map, the scene depth map and the ground truth image.
* 11 image grid (cases 6 and 7) has the 11 image grid consists of the image without shadows, the light map, 8 view depth maps and the ground truth image.
