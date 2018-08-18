#pragma once
#include <opencv2/opencv.hpp>
#include "CameraApi.h"
#include "CameraDefine.h"
#include <semaphore.h>

using namespace cv;


typedef void *PVOID;
//#define far
//typedef void far *LPVOID;

// camear num
#define CAMERA_NUM 10
// 
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
		sem_wait(&sems);
		return srcImage;
	}


	BOOL            m_bExit = FALSE;
	CameraHandle    m_hCamera;			
	BYTE*           m_pFrameBuffer; 
	tSdkFrameHead   m_sFrInfo;		
	Mat srcImage;
	sem_t           sems;

private:
	UINT            m_threadID;		
	
	HANDLE          m_hDispThread;	
	int	            m_iDispFrameNum;	
	float           m_fDispFps;			
	float           m_fCapFps;			
	tSdkFrameStatistic  m_sFrameCount;
	tSdkFrameStatistic  m_sFrameLast;
	int					m_iTimeLast;
	char		    g_CameraName[64];
	pthread_t       id;
};

