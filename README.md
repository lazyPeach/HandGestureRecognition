==============================
Hand Gesture Recognition
==============================

Description:

  The purpose of this application is to detect hand gestures such as open hand, closed hand, or finger up etc. 
  Application uses openCV library mainly for easier access to a .jpeg file or web cam manipulation. There will be no other openCV specific methods for image manipulation. 
  The main idea is to find the mass center of the hand and the concavity/convexity points. Based on this information one should be able to draw some conclusions related to the hand gesture represented.

------------------------------

Build and execution:

  Application is developed using Visual Studio 2012. The setup was done according to openCV official
  site.

------------------------------

Algorithm description:

  For simplicity, the applicaiton does not perform hand detection but only hand gesture recognition. One simple way to get the hand from an image is to have a background image which will be subtracted from further images. As a result of the subtraction, most of the pixels will have value 0 since the background doesn't change and the points representing the hand will have value different from 0.
  
  In the real-case scenario (using a webcam) we will use the first frame as background reference which will be subtracted from all other frames. Other approaches would be to detect the objects that have skin color and remove the head from the image but that would involve more work and computation overhead.
  
  After background subtraction, the only objects remaining in the image are the hand and some noise. Usually the hand is the biggest object remaining in the image so we can remove noise based on this assumption.

  Firstly, we need to binarize the image. Secondly we will apply some morphological transformations (erosion and dilation) for closing the gaps that might appear in the hand object. Then we pass the entire image to get all objects and store their entry point (first point discovered), their area and their assigned label.

  Using the object having the biggest area, we will determine its weight center point. Then we will compute the convex hull that encompases the object. After that we search for convexity defects and consider the points in the convex hull between which there was a big enough convexity defect to be the extremity of a finger.
