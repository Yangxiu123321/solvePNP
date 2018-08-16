#pragma once
#include "windows.h"
#include <opencv2/opencv.hpp>
#include <process.h>
#include "..//include//CameraApi.h"

using namespace cv;

typedef void *PVOID;

// 定义相机的个数
#define CAMERA_NUM 10
// 定义图片尺寸
#define IMAGE_ROWS 480
#define IMAGE_COLS 640

class MvInit
{
public:
	MvInit();
	~MvInit();
	CameraSdkStatus createCamera();

	Mat getImage() 
	{
		WaitForSingleObject(g_hEventBufferFull, INFINITE);
		return srcImage;
	}


	BOOL            m_bExit = FALSE;//用来通知图像抓取线程结束
	CameraHandle    m_hCamera;		//相机句柄，多个相机同时使用时，可以用数组代替	
	BYTE*           m_pFrameBuffer; //用于将原始图像数据转换为RGB的缓冲区
	tSdkFrameHead   m_sFrInfo;		//用于保存当前图像帧的帧头信息
	HANDLE       g_hEventBufferFull;//用于创建线程
	Mat srcImage;
private:
	UINT            m_threadID;		//图像抓取线程的ID
	
	HANDLE          m_hDispThread;	//图像抓取线程的句柄
	int	            m_iDispFrameNum;	//用于记录当前已经显示的图像帧的数量
	float           m_fDispFps;			//显示帧率
	float           m_fCapFps;			//捕获帧率
	tSdkFrameStatistic  m_sFrameCount;
	tSdkFrameStatistic  m_sFrameLast;
	int					m_iTimeLast;
	char		    g_CameraName[64];
};

