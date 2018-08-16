// OpenCV.cpp : 定义控制台应用程序的入口点。
//
//BIG5 TRANS ALLOWED
#include "windows.h"
#include <opencv2/opencv.hpp>
#include <process.h>
#include "mv_Init.h"
#include "PNPSolver.h"
#include "..//include//CameraApi.h"

using namespace std;
using namespace cv;


int main(int argc, char* argv[])
{
	MvInit camera;
	PNPSolver solvePnP(1949.4,1960.3,714.68,508.05,-0.2376,0.2338,1.9481e-4,0.0022,0.0);
	Mat srcImage;
	namedWindow("src");
	while (camera.m_bExit != TRUE)
	{
		srcImage = camera.getImage();
		imshow("src",srcImage);

		int c = waitKey(10);
		if (c == 'q' || c == 'Q' || (c & 255) == 27)
		{
			camera.m_bExit = TRUE;
			break;
		}
	}
	return 0;
}

