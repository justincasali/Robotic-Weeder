import cv2
import numpy as np
import math

scale = 13.75/30
y_origin = 480 / 2
x_origin = 640 / 2

def coord_scale(x_array, y_array):
    
    x_return = list()
    y_return = list()

    for x in x_array:
        x_return.append(round((x - x_origin) * (1 + scale) + x_origin)-2)

    for y in y_array:
        y_return.append(round((y - y_origin) * (1 + scale) + y_origin)-2)

    return x_return, y_return

def get_red():
    #capture image
    camera = cv2.VideoCapture(0)
    
    #rotate in orientation with actuator
    ret, image = camera.read()
    rows, cols, useless = image.shape
    rotate = cv2.getRotationMatrix2D((cols/2,rows/2),180,1)
    image = cv2.warpAffine(image,rotate,(cols,rows))

    #Blur image
    blur = cv2.GaussianBlur(image,(5,5),0)

    #red thresholds
    lower_red = np.array([0,0,150])
    upper_red = np.array([175,175,255])

    #check color thesholds	
    data = cv2.inRange(image, lower_red, upper_red)
	
    #convert to binary image
    binImg = cv2.bitwise_and(data, data)

    #find contours
    edges, contours, hierarchy = cv2.findContours(binImg,cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)

    #filter contours
    newContours = []
    for i in range(0,len(contours)):
        if (cv2.contourArea(contours[i]) > 750 and cv2.contourArea(contours[i]) < 1500):
            arearadius = math.sqrt(cv2.contourArea(contours[i]) / math.pi)
            perimeterradius = cv2.arcLength(contours[i], True) / (2 * math.pi)
            if (arearadius + 10 > perimeterradius and arearadius -10 < perimeterradius):
                newContours.append(contours[i])
    # init x, y
    x = -1
    y = -1
    
    #find centers, put in arrays
    for c in newContours:
	    obj = cv2.moments(c) 	
	    x = int(obj["m10"] / obj["m00"])
	    y = int(obj["m01"] / obj["m00"])
	    
    return x, y

def dandelions_exist():
    #capture image
    camera = cv2.VideoCapture(0)
    
    #rotate image
    ret, image = camera.read()
    rows, cols, useless = image.shape
    rotate = cv2.getRotationMatrix2D((cols/2,rows/2),180,1)
    image1 = cv2.warpAffine(image,rotate,(cols,rows))
    
    #reduce noise
    blur = cv2.GaussianBlur(image,(5,5),0)
	
	#convert bgr to hsv
    hsv = cv2.cvtColor(blur,cv2.COLOR_BGR2HSV)
	
	#white thresholds
    lower_white = np.array([0,0,200])
    upper_white = np.array([390,255,255])

	#yellow thresholds
    lower_yellow = np.array([20,75,75])
    upper_yellow = np.array([30,255,255])

	#check color thesholds	
    data = cv2.inRange(hsv,lower_white, upper_white) + cv2.inRange(hsv,lower_yellow,upper_yellow)
	
	#convert to binary image
    binImg = cv2.bitwise_and(data, data, data)
	
	#find contours
    edges, contours, hierarchy = cv2.findContours(binImg,cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)
    
    newContours = []
    for x in range(0,len(contours)):
        if cv2.contourArea(contours[x]) > 500 and cv2.contourArea(contours[x]) < 1500 :
            arearadius = math.sqrt(cv2.contourArea(contours[x]) / math.pi)
            perimeterradius = cv2.arcLength(contours[x], True) / (2 * math.pi)
            if (arearadius + 10 > perimeterradius and arearadius -10 < perimeterradius):
                newContours.append(contours[x])
    

    if len(newContours) != 0:
        return True
    elif len(newContours) == 0:
        return False

def locate():
    #capture image
    camera = cv2.VideoCapture(0)
    
    #rotate image
    ret, image = camera.read()
    rows, cols, useless = image.shape
    rotate = cv2.getRotationMatrix2D((cols/2,rows/2),180,1)
    image1 = cv2.warpAffine(image,rotate,(cols,rows))
    
    #reduce noise
    blur = cv2.GaussianBlur(image1,(5,5),0)
	
	#convert bgr to hsv
    hsv = cv2.cvtColor(blur,cv2.COLOR_BGR2HSV)
	
	#white thresholds
    lower_white = np.array([0,0,200])
    upper_white = np.array([390,255,255])

	#yellow thresholds
    lower_yellow = np.array([20,75,75])
    upper_yellow = np.array([30,255,255])

	#check color thesholds	
    data = cv2.inRange(hsv,lower_white, upper_white) + cv2.inRange(hsv,lower_yellow,upper_yellow)
	
	#convert to binary image
    binImg = cv2.bitwise_and(data, data, data)
	
	#find contours
    edges, contours, hierarchy = cv2.findContours(binImg,cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)
    
    newContours = []
    for x in range(0,len(contours)):
        if cv2.contourArea(contours[x]) > 500 and cv2.contourArea(contours[x]) < 1500 :
            arearadius = math.sqrt(cv2.contourArea(contours[x]) / math.pi)
            perimeterradius = cv2.arcLength(contours[x], True) / (2 * math.pi)
            if (arearadius + 10 > perimeterradius and arearadius -10 < perimeterradius):
                newContours.append(contours[x])
            
    '''cv2.imshow('Contours',edges)
    img = cv2.drawContours(image, newContours, -1, (0,255,0), 3)
    cv2.imshow('NewContours',img)
    cv2.waitKey(0)
    cv2.destroyAllWindows()'''
    
    x_array = []
    y_array = []
    
    #find centers, put in arrays
    for c in newContours:
	    obj = cv2.moments(c) 	
	    x = int(obj["m10"] / obj["m00"])
	    y = int(obj["m01"] / obj["m00"])
	    
	    # make sure projected dandelions are in range
	    if (x > 100 and x < 540 and y > 75 and y < 405): 
	        x_array.append(x)
	        y_array.append(y)
     
    return coord_scale(x_array, y_array)
    
    
def verify(x, y):
    thresh = 20
    x_red, y_red = get_red()
    if abs(x - x_red) <= thresh and abs(y - y_red) <= thresh:
        return True
    else:
        return False

