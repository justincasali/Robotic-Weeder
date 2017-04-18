import cv2
import numpy as np
import math
import serial

RX_COORD = 0x01
TX_CAL = 0x01

micro = serial.Serial('/dev/ttyUSB0')

def wait_for_request():
    while True:
        if (micro.in_waiting > 0):
            if (micro.read() == bytes([RX_COORD])):
                break

def get_coord():
	camera = cv2.VideoCapture(0)
	ret, image = camera.read()
	rows, cols, useless = image.shape
	rotate = cv2.getRotationMatrix2D((cols/2,rows/2),180,1)
	image = cv2.warpAffine(image,rotate,(cols,rows))
	blur = cv2.GaussianBlur(image,(5,5),0)

	#red thresholds
	lower_red = np.array([100,50,250])
	upper_red = np.array([225,175,255])

	#check color thesholds	
	data = cv2.inRange(image, lower_red, upper_red)
	
	#convert to binary image
	binImg = cv2.bitwise_and(data, data, data)

	#find contours
	edges, contours, hierarchy = cv2.findContours(binImg,cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)
	
	#filter contours
	newContours = []
	for x in range(0,len(contours)):
		if cv2.contourArea(contours[x]) > 100 and cv2.contourArea(contours[x]) < 3000 :
			arearadius = math.sqrt(cv2.contourArea(contours[x]) / math.pi)
			perimeterradius = cv2.arcLength(contours[x], True) / (2 * math.pi)
	
			if arearadius + 10 > perimeterradius and arearadius -10 < perimeterradius:
				newContours.append(contours[x])

	#intialize center arrays
	Cx = []
	Cy = []

	#find centers, put in arrays
	for c in newContours:
		obj = cv2.moments(c) 	
		cx = int(obj["m10"] / obj["m00"])
		cy = int(obj["m01"] / obj["m00"])
		img = cv2.circle(image,(cx,cy),3,(0,0,255),-1)
		Cx.append(cx)
		Cy.append(cy)

	xavg = int(np.mean(Cx))
	yavg = int(np.mean(Cy))
	print('Location:', xavg, yavg)
	
	return bytes([xavg >> 8 & 0xff, xavg & 0xff, yavg >> 8 & 0xff, yavg & 0xff])


# Send go to coord state byte
micro.write(bytes([TX_CAL]))

# Sends data for each calibration coordinate
for x in range(4):
	wait_for_request()
	micro.reset_input_buffer()
	coord = get_coord()
	micro.write(coord)
	print('Sent:', coord)

