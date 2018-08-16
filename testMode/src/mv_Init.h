#pragma once
#include "windows.h"
#include <opencv2/opencv.hpp>
#include <process.h>
#include "..//include//CameraApi.h"

using namespace cv;

typedef void *PVOID;

// ��������ĸ���
#define CAMERA_NUM 10
// ����ͼƬ�ߴ�
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


	BOOL            m_bExit = FALSE;//����֪ͨͼ��ץȡ�߳̽���
	CameraHandle    m_hCamera;		//��������������ͬʱʹ��ʱ���������������	
	BYTE*           m_pFrameBuffer; //���ڽ�ԭʼͼ������ת��ΪRGB�Ļ�����
	tSdkFrameHead   m_sFrInfo;		//���ڱ��浱ǰͼ��֡��֡ͷ��Ϣ
	HANDLE       g_hEventBufferFull;//���ڴ����߳�
	Mat srcImage;
private:
	UINT            m_threadID;		//ͼ��ץȡ�̵߳�ID
	
	HANDLE          m_hDispThread;	//ͼ��ץȡ�̵߳ľ��
	int	            m_iDispFrameNum;	//���ڼ�¼��ǰ�Ѿ���ʾ��ͼ��֡������
	float           m_fDispFps;			//��ʾ֡��
	float           m_fCapFps;			//����֡��
	tSdkFrameStatistic  m_sFrameCount;
	tSdkFrameStatistic  m_sFrameLast;
	int					m_iTimeLast;
	char		    g_CameraName[64];
};

