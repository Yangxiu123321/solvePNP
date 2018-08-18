#include <iostream>

#include "opencv2/opencv.hpp"

#include "apriltag.h"
#include "tag36h11.h"
#include "tag36h10.h"
#include "tag36artoolkit.h"
#include "tag25h9.h"
#include "tag25h7.h"
#include "common/getopt.h"

#include "mv_Init.h"
#include "aprilTags.h"
#include "PNPSolver.h"

using namespace std;
using namespace cv;

int main(int argc,char *argv[])
{
    Mat frame, gray;
    namedWindow("src");
    // aprilTag初始化
    AprilTags april(argc,argv);
    // 相机初始化
    MvInit camera;
    //solvePnp 初始化
    PNPSolver solvePNP(1153.0681,1159.3143,340.6709,217.9774,-0.2756,-0.2038,0.0020,0.0014);
    while(camera.m_bExit != TRUE)
    {
        frame = camera.getImage();

        if(frame.empty())
        {
            cerr<<"no picture"<<endl;
            return 0;
        }
        april.inputSrcImg(frame);
        imshow("src",frame);

        cvtColor(frame, gray, COLOR_BGR2GRAY);
        // Make an image_u8_t header for the Mat data
        image_u8_t im  = { .width = gray.cols,
            .height = gray.rows,
            .stride = gray.cols,
            .buf = gray.data
        };
        // 识别Tag
        april.targetDetect(&im);
 
        // 得到四个角点，框出Tag
        april.drawLine();

        // 解位姿
        solvePNP.applySolvePNP(april.targetPoints);
        
        
        int c = waitKey(30);
		if (c == 'q' || c == 'Q' || (c & 255) == 27)
		{
			camera.m_bExit = TRUE;
			break;
		}
        // waitKey(0);
    }

    destroyWindow("src");
    return 0;
}