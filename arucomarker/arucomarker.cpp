// arucomarker.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "kinect.h"
#include <iostream>
#include <opencv2/core/core.hpp>  
#include <opencv2/highgui/highgui.hpp>
#include<opencv2/opencv.hpp>
#include <time.h>
#include <vector>
#include <opencv2/aruco.hpp>  

using namespace std;
using namespace cv;

const Mat cameraMatrix = (Mat_<float>(3, 3) << 1083.148343406121285, 0, 966.490110494402870,
	0, 1084.264408691762355, 535.157215951467947,
	0, 0, 1);
const Mat distCoeffs = (Mat_<float>(5, 1) << 0.074351096544866, -0.099292370734144, -0.002268994919990,
	0.001492042893174, 0.000000000000000);

template<class Interface>
inline void SafeRelease(Interface *& pInterfaceToRelease)
{
	if (pInterfaceToRelease != NULL)
	{
		pInterfaceToRelease->Release();
		pInterfaceToRelease = NULL;
	}
}

int main() {

	//marker
	/*Mat markerImage;
	cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_250);
	cv::aruco::drawMarker(dictionary, 23, 200, markerImage, 1);
	imwrite("marker.jpg", markerImage);*/
	
	IKinectSensor* m_pKinectSensor;
	HRESULT hr;

	//sensor
	hr = GetDefaultKinectSensor(&m_pKinectSensor);
	if (FAILED(hr)) {
		std::cerr << "Error : GetDefaultKinectSensor" << std::endl;
	}
	hr=m_pKinectSensor->Open();
	if (FAILED(hr)) {
		std::cerr << "Error : IKinectSensor::Open()" << std::endl;
	}
	
	//source
	IColorFrameSource* pColorSource; 
	hr = m_pKinectSensor->get_ColorFrameSource(&pColorSource);
	if (FAILED(hr)) {
		std::cerr << "Error : IKinectSensor::get_ColorFrameSource()" << std::endl;
	}

	//reader
	IColorFrameReader* pColorReader;
	hr = pColorSource->OpenReader(&pColorReader); 
	if (FAILED(hr)) {
		std::cerr << "Error : IColorFrameSource::OpenReader()" << std::endl;
	}
	
	//data
	int width = 1920;
	int height = 1080;
	Mat bufferMat(height, width, CV_8UC4);
	Mat colorMat(height / 2, width / 2, CV_8UC4);
	//cv::namedWindow("Color");
	UINT nColorBufferSize = 1920 * 1080 * sizeof(RGBQUAD);
	IColorFrame* pColorFrame = NULL;
	while (1) {
		// Frame
		IColorFrame* pColorFrame = NULL;
		hr = pColorReader->AcquireLatestFrame(&pColorFrame);
		if (SUCCEEDED(hr)) {
			hr = pColorFrame->CopyConvertedFrameDataToArray(nColorBufferSize, reinterpret_cast<BYTE*>(bufferMat.data), ColorImageFormat::ColorImageFormat_Bgra);
		}
		resize(bufferMat, colorMat, Size(), 0.5, 0.5);
		imshow("color", colorMat);
		Mat gray,graycopy;
		cvtColor(colorMat, gray, CV_BGRA2BGR);
		gray.copyTo(graycopy);
		std::vector<int> markerIds;
		std::vector<std::vector<cv::Point2f>> markerCorners, rejectedCandidates;
		cv::Ptr<cv::aruco::DetectorParameters> parameters= aruco::DetectorParameters::create();
		cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_250);
		cv::aruco::detectMarkers(gray, dictionary, markerCorners, markerIds, parameters, rejectedCandidates);
		cv::Mat outputImage;
		cout << markerCorners.size() << endl;
		cv::aruco::drawDetectedMarkers(graycopy, markerCorners, markerIds);		std::vector<cv::Vec3d> rvecs, tvecs;		cv::aruco::estimatePoseSingleMarkers(markerCorners, 0.05, cameraMatrix, distCoeffs, rvecs, tvecs);		for (int i = 0; i<markerIds.size(); i++)
			cv::aruco::drawAxis(graycopy, cameraMatrix, distCoeffs, rvecs[i], tvecs[i], 0.1);		imshow("ppp", graycopy);		if (cv::waitKey(1) == VK_ESCAPE) {			break;		}
		SafeRelease(pColorFrame);
	}
	destroyAllWindows();
	m_pKinectSensor->Close();
	std::system("pause");
	return 0;
}