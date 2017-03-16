#include <opencv2/highgui/highgui.hpp>
#include <opencv2/video/tracking.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include "opencv2/imgcodecs.hpp"
#include "opencv2/videoio.hpp"
#include <opencv2/video.hpp>

#include <string>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <sstream>

cv::Rect rectMax(std::vector<cv::Rect> rects)
{
	if(rects.size() == 0)
	{
		return cv::Rect();
	}
	cv::Rect boundingBox = rects[0];
	for(int i = 1; i < rects.size(); ++i )
	{
		if(rects[i].height > boundingBox.height && rects[i].width > boundingBox.width)
		{
			boundingBox = rects[i];
		}
	}

	if(boundingBox.width <= 20 || boundingBox.height <= 40)
	{
		boundingBox = cv::Rect();
	}

	return boundingBox;

}

cv::Rect processForeground(cv::Mat foreground)
{
	cv::Mat kernel = cv::Mat::ones(cv::Size(2,2), CV_32F);
	std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Vec4i> hierarchy;


	cv::erode(foreground, foreground,kernel,cv::Point(-1,-1),1);
	cv::dilate(foreground, foreground,kernel,cv::Point(-1,-1),10);

	cv::findContours( foreground, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0) );
	std::vector<std::vector<cv::Point> > contours_poly( contours.size() );
	std::vector<cv::Rect> boundRect( contours.size() );

	for(int i = 0; i < contours.size(); ++i)
	{
		cv::approxPolyDP( cv::Mat(contours[i]), contours_poly[i], 3, true );
		boundRect[i] = cv::boundingRect( cv::Mat(contours_poly[i]) );
	}

	cv::Rect boundingBox;
	boundingBox = rectMax(boundRect);

	return boundingBox;
}

std::pair<cv::Mat, cv::Rect> preProcessFrame(cv::Mat frame, cv::Mat foreground)
{
	std::pair<cv::Mat, cv::Rect> frameRect;
	cv::Mat frameSegmented;

	cv::HOGDescriptor hog;
	hog.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());
	cv::Ptr<cv::BackgroundSubtractorMOG2> pMOG2 = cv::createBackgroundSubtractorMOG2(5, 8, false);

	std::vector<cv::Rect> human;
	hog.detectMultiScale(frame, human, 0, cv::Size(8,8), cv::Size(32,32), 1.05, 0);
	cv::Rect boundingBox;

	if (human.size() > 0) {
		boundingBox = rectMax(human);
	}
	else
	{
		boundingBox = processForeground(foreground);
	}

	if(boundingBox.area() != 0)
	{
		if((boundingBox.x + 64) < frame.cols)
		{
			boundingBox.width = 64;
			boundingBox.y = 0;
			boundingBox.height = 120;
//			frameSegmented = frame(boundingBox);
		}
		else
		{
			boundingBox.width = 64;
			boundingBox.y = 0;
			boundingBox.height = 120;
			boundingBox.x = frame.cols - 64;
		}
			frameSegmented = frame(boundingBox);

	}

	frameRect.first = frameSegmented;
	frameRect.second = boundingBox;

	return frameRect;
}

cv::Mat computeOpticalFlow(cv::Mat prevFrame, cv::Mat currentFrame)
{
	cv::Mat flow;

	cv::calcOpticalFlowFarneback(prevFrame, currentFrame, flow, 0.5, 3, 15, 3, 5, 1.2, 0);

	return flow;
}

std::pair<cv::Mat, cv::Mat> separeteFlowXY(cv::Mat flow)
{

	std::pair<cv::Mat, cv::Mat> flowXY;

	cv::Mat flowX = cv::Mat::zeros(flow.rows, flow.cols, CV_32FC1);
	cv::Mat flowY = cv::Mat::zeros(flow.rows, flow.cols, CV_32FC1);

	for(int i = 0; i < flow.rows; ++i)
	{
		for(int j = 0; j < flow.cols; ++j)
		{
			cv::Point2f point = flow.at<cv::Point2f>(i,j);
			flowX.at<float>(i,j) = point.x;
			flowY.at<float>(i,j) = point.y;
		}
	}

	flowXY.first = flowX;
	flowXY.second = flowY;

	return flowXY;
}

std::pair<cv::Mat, cv::Mat> computeGradients(cv::Mat currentFrame)
{
	GaussianBlur( currentFrame, currentFrame, cv::Size(3,3), 0, 0, cv::BORDER_DEFAULT );

	std::pair<cv::Mat, cv::Mat> gradients;

	cv::Mat gradX, gradY;
	cv::Sobel( currentFrame, gradX, CV_16S, 1, 0, 3, 1, 0, cv::BORDER_DEFAULT );
	cv::Sobel( currentFrame, gradY, CV_16S, 0, 1, 3, 1, 0, cv::BORDER_DEFAULT );

	gradients.first = gradX;
	gradients.second = gradY;

	return gradients;
}

cv::Mat drawFlow(cv::Mat frame, cv::Mat flow)
{
	cv::Mat frameDrawed;
	frame.copyTo(frameDrawed);

	for (int y = 0; y < frame.rows; y += 5)
	{
		for (int x = 0; x < frame.cols; x += 5)
		{
			// get the flow from y, x position * 10 for better visibility
			const cv::Point2f flowatxy = flow.at<cv::Point2f>(y, x) * 10;
			// draw line at flow direction
			line(frameDrawed, cv::Point(x, y), cv::Point(cvRound(x + flowatxy.x), cvRound(y + flowatxy.y)), cv::Scalar(255,0,0));
			// draw initial point
			circle(frameDrawed, cv::Point(x, y), 1, cv::Scalar(0, 0, 0), -1);
		}
	}

	return frameDrawed;
}

cv::Mat drawRect(cv::Mat currentFrame, cv::Rect boundingbox)
{

	cv::Mat rectDrawed;

	currentFrame.copyTo(rectDrawed);

	cv::rectangle(rectDrawed, boundingbox.tl(), boundingbox.br(), cv::Scalar(0,255,0), 3);

	return rectDrawed;

}

std::vector<std::string> split( std::string str, char sep = ' ' )
{
	std::vector<std::string> ret ;

	std::istringstream stm(str) ;
	std::string token ;
	while( std::getline( stm, token, sep ) ) ret.push_back(token) ;

	return ret ;
}

void saveFeatures(std::string videoPath, std::string pathDirOut, std::string dirVideo, std::string index,
		std::pair<cv::Mat, cv::Mat> flowXY, std::pair<cv::Mat, cv::Mat> gradients, cv::Mat grayscaleFrame)
{
		std::vector<std::string> tokens = split(videoPath, '/');

		std::string videoName = tokens[tokens.size()-1];
		std::string label = tokens[tokens.size()-2];

		cv::imwrite(pathDirOut + "/grayscale/" + label + "/"+ dirVideo + "/" + index + ".png", grayscaleFrame);
		cv::imwrite(pathDirOut + "/grad_x/" + label + "/"+ dirVideo + "/" + index + ".png", gradients.first);
		cv::imwrite(pathDirOut + "/grad_y/" + label + "/"+ dirVideo + "/" + index + ".png", gradients.second);

		if(!flowXY.first.empty() && !flowXY.second.empty())
		{
			std::string fileNameFlowX = pathDirOut + "/flow_x/" + label + "/"+ dirVideo + "/" + index + ".yaml";
			std::string fileNameFlowY = pathDirOut + "/flow_y/" + label + "/"+ dirVideo + "/" + index + ".yaml";

			cv::FileStorage storageX(fileNameFlowX, cv::FileStorage::WRITE);
			storageX << "flow_x" << flowXY.first;
			storageX.release();

			cv::FileStorage storageY(fileNameFlowY, cv::FileStorage::WRITE);
			storageY << "flow_y" << flowXY.second;
			storageY.release();
		}
}

std::string intToString (int a)
{
	std::ostringstream temp;
	temp<<a;
	return temp.str();
}

int main(int argc, char **argv)
{
	cv::VideoCapture cap(argv[1]);

	cv::Mat curretFrame, prevFrame, foreground;

	cv::HOGDescriptor hog;
	hog.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());
	cv::Ptr<cv::BackgroundSubtractorMOG2> pMOG2 = cv::createBackgroundSubtractorMOG2(5, 8, false);

	int count = 0;
	for(;;)
	{
		cap >> curretFrame;
		if(curretFrame.empty()) break;

		cv::cvtColor( curretFrame, curretFrame, CV_BGR2GRAY );

		cv::Mat frameResized;
		cv::resize(curretFrame, frameResized, cv::Size(160, 120));

		pMOG2->apply(frameResized, foreground);

		std::pair<cv::Mat, cv::Rect>  segmentedFrameAndRect = preProcessFrame(frameResized, foreground);
		cv::Mat segmentedFrame = segmentedFrameAndRect.first;
		if(segmentedFrame.empty()) continue;

		std::pair<cv::Mat, cv::Mat> gradients = computeGradients(segmentedFrame);
		std::pair<cv::Mat, cv::Mat> flowXY;
		cv::Mat flow, flowDrawed;
		if(!prevFrame.empty())
		{
			flow = computeOpticalFlow(prevFrame, segmentedFrame);
			flowXY = separeteFlowXY(flow);
			flowDrawed = drawFlow(segmentedFrame, flow);
		}

//		saveFeatures(argv[1], argv[2], argv[3], intToString(count) , flowXY, gradients, segmentedFrame);
		count +=1;


		cv::Mat rectDrawed = drawRect(curretFrame, segmentedFrameAndRect.second);

		cv::Mat gradX, gradY;
		gradX = gradients.first;
		gradY = gradients.second;
		gradX.convertTo(gradX, CV_8UC1);
		gradY.convertTo(gradY, CV_8UC1);
		cv::imshow("original-frame", curretFrame);
		cv::imshow("person-detected", rectDrawed);
		cv::imshow("preprocessed-frame", segmentedFrame);
		if(!flowDrawed.empty())
			cv::imshow("flow", flowDrawed);
		cv::imshow("Sobel-x", gradX);
		cv::imshow("Sobel-y", gradY);

		if(cv::waitKey(20) >= 0) break;

		unsigned int microseconds = 1000000;
		usleep(microseconds);

		segmentedFrame.copyTo(prevFrame);
	}

	return 0;
}



/**
 * void processVideo(cv::VideoCapture cap)
{

	cv::Mat frame, foreground;
	cv::Ptr<cv::BackgroundSubtractorMOG2> pMOG2 = cv::createBackgroundSubtractorMOG2(5, 8, false);
	cv::Mat kernel = cv::Mat::ones(cv::Size(2,2), CV_32F);
	std::vector<std::vector<cv::Point> > contours;
	std::vector<cv::Vec4i> hierarchy;
	for(;;)
	{
		cap >> frame;
		if(frame.empty()) break;

		pMOG2->apply(frame, foreground);

		cv::erode(foreground, foreground,kernel,cv::Point(-1,-1),1);
		cv::dilate(foreground, foreground,kernel,cv::Point(-1,-1),10);

		cv::findContours( foreground, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0) );
		std::vector<std::vector<cv::Point> > contours_poly( contours.size() );
		std::vector<cv::Rect> boundRect( contours.size() );

		for(int i = 0; i < contours.size(); ++i)
		{
			cv::approxPolyDP( cv::Mat(contours[i]), contours_poly[i], 3, true );
			boundRect[i] = cv::boundingRect( cv::Mat(contours_poly[i]) );
		}

		if(boundRect.size() > 0)
		{
			cv::Rect boundingBox = rectMax(boundRect);
			if(boundingBox.width > 30 && boundingBox.height > 50)
			{
				rectangle(frame, boundingBox.tl(), boundingBox.br(), cv::Scalar(0,255,0), 2, 8, 0);
			}
		}
		cv::imshow("Frame", frame);
		cv::imshow("foreground-erode", foreground);
		if(cv::waitKey(20) >= 0) break;
	}
}
 */

