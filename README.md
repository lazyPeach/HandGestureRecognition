HandGestureRecognition
==============================

Basic hand gestures recognition.

==============================

Description:
  Application will detect basic hand gestures such as open hand, closed hand, on finger up etc. 
  Application uses openCV library mainly for easier access to a .jpeg file or web cam manipulation. 
  There will be no other openCV specific methods for image manipulation. The main idea is to find 
  the mass center of the hand and the concavity/convexity points. Based on this information one 
  should be able to draw some conclusions related to the hand gesture.

Build and execution:
  Application is developed using Visual Studio 2012. The setup was done according to openCV official
  site.

Algorithm description:

  For simplicity, the applicaiton does not perform hand detection bu only hand gesture recognition. 
  One simple way to get the hand from an image is to have the background of the image and subtract 
  it. For the real-case scenario (using a webcam) this means that we will use the first frame as 
  background reference which will be subtracted from all other frames. This approach simplifies the 
  algorithm alot. An appproach for detecting only the hand would be to binarize the image by skin 
  color, get the 2 biggest objects in the image (usually the head and the hand) and check which one 
  of them has holes. Usually, after binarization, the eyes and mouth appear as holes into the face. 
  After this step the head should be removed and only the hand area will be of interest. This 
  approach somplicates a little bit the algorithm because if there is only face and no hand in the 
  image, the biggest 2 objects will be hand and another one, probably some noise from background 
  which could not have holes and it would be more difficult to extract the hand. Thus for the sake 
  of simplicity, we will use the first frame as background reference and subtract it from the others.
  This approach also has drawbacks is the background changes during the application running but this
  is unlikely.

  After we subtract the background, we will perform some transformations on the image. First one of 
  them will be to transform it from RGB space to HSV one to be less sensitive to light change. After
  that the image will be binarized and a series of erosions and dilations will be performed to reduce
  noise. Ideally, after this step, only the hand will be white in the image and the rest will be 
  black. Still this is not guaranteed because there might be some noise so the image will be traced 
  again for finding the biggest object (handl). When we have the hand, we should find the center of 
  mass and using a contour tracing algorithm the points of convexity and concavity. 

  Based on the distance from center of mass and these points and the angles between these points we
  should be able to identify the hand gesture. 
