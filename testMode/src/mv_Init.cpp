#include "mv_Init.h"
#include "windows.h"
#include <iostream>
#include <opencv2/opencv.hpp>
#include <process.h>
#include "..//include//CameraApi.h"

using namespace std;

MvInit::MvInit()
{
	createCamera();
}


MvInit::~MvInit()
{
	CameraUnInit(m_hCamera);

	CameraAlignFree(m_pFrameBuffer);

	destroyWindow(g_CameraName);

	CloseHandle(g_hEventBufferFull);
}

UINT WINAPI uiDisplayThread(LPVOID lpParam)
{
	tSdkFrameHead 	sFrameInfo;

	MvInit          *pThis = (MvInit*)lpParam;
	BYTE*			pbyBuffer;
	IplImage *iplImage = NULL;
	CameraSdkStatus status = CAMERA_STATUS_SUCCESS;

	while (!pThis->m_bExit)
	{

		if (CameraGetImageBuffer(pThis->m_hCamera, &sFrameInfo, &pbyBuffer, 1000) == CAMERA_STATUS_SUCCESS)
		{
			//将获得的原始数据转换成RGB格式的数据，同时经过ISP模块，对图像进行降噪，边沿提升，颜色校正等处理。
			//我公司大部分型号的相机，原始数据都是Bayer格式的
			status = CameraImageProcess(pThis->m_hCamera, pbyBuffer, pThis->m_pFrameBuffer, &sFrameInfo);//连续模式

																						 //分辨率改变了，则刷新背景
			if (IMAGE_COLS != sFrameInfo.iWidth || IMAGE_ROWS != sFrameInfo.iHeight)
			{
				cout << "cols:" << sFrameInfo.iWidth << "rows" << sFrameInfo.iHeight << endl;
				exit(0);
			}
			else
			{
				// 获得相机的参数信息
				memcpy(&pThis->m_sFrInfo, &sFrameInfo, sizeof(tSdkFrameHead));
			}

			if (status == CAMERA_STATUS_SUCCESS)
			{
				//调用SDK封装好的显示接口来显示图像,您也可以将m_pFrameBuffer中的RGB数据通过其他方式显示，比如directX,OpengGL,等方式。
				//CameraImageOverlay(hCamera, m_pFrameBuffer, &sFrameInfo);

				cv::Mat matImage(
					cvSize(sFrameInfo.iWidth, sFrameInfo.iHeight),
					sFrameInfo.uiMediaType == CAMERA_MEDIA_TYPE_MONO8 ? CV_8UC1 : CV_8UC3,
					pThis->m_pFrameBuffer
				);
				matImage.copyTo(pThis->srcImage);

				SetEvent(pThis->g_hEventBufferFull);
			}

			//在成功调用CameraGetImageBuffer后，必须调用CameraReleaseImageBuffer来释放获得的buffer。
			//否则再次调用CameraGetImageBuffer时，程序将被挂起，知道其他线程中调用CameraReleaseImageBuffer来释放了buffer
			CameraReleaseImageBuffer(pThis->m_hCamera, pbyBuffer);
		}
	}

	if (iplImage)
	{
		cvReleaseImageHeader(&iplImage);
	}
	_endthreadex(0);
	return 0;
}

 CameraSdkStatus MvInit::createCamera()
{
	 tSdkCameraDevInfo sCameraList[CAMERA_NUM];
	 INT iCameraNums;
	 CameraSdkStatus status = CAMERA_STATUS_SUCCESS;
	 tSdkCameraCapbility sCameraInfo;

	 //创建二个自动复位事件，一个表示缓冲区是否为空，另一个表示缓冲区是否已经处理
	 g_hEventBufferFull = CreateEvent(NULL, FALSE, FALSE, NULL);

	 //枚举设备，获得设备列表
	 iCameraNums = CAMERA_NUM;//调用CameraEnumerateDevice前，先设置iCameraNums = 10，表示最多只读取10个设备，如果需要枚举更多的设备，请更改sCameraList数组的大小和iCameraNums的值

	 if (CameraEnumerateDevice(sCameraList, &iCameraNums) != CAMERA_STATUS_SUCCESS || iCameraNums == 0)
	 {
		 printf("No camera was found!");
		 return FALSE;
	 }


	// 初始化相机
	if ((status = CameraInit(&sCameraList[0], -1, -1, &m_hCamera)) != CAMERA_STATUS_SUCCESS)
	{
		char msg[128];
		sprintf_s(msg, "Failed to init the camera! Error code is %d", status);
		printf(msg);
		printf(CameraGetErrorString(status));
		return FALSE;
	}

	//"获得该相机的特性描述"
	CameraGetCapability(m_hCamera, &sCameraInfo);

	m_pFrameBuffer = (BYTE *)CameraAlignMalloc(sCameraInfo.sResolutionRange.iWidthMax*sCameraInfo.sResolutionRange.iWidthMax * 3, 16);


	if (sCameraInfo.sIspCapacity.bMonoSensor)
	{
		CameraSetIspOutFormat(m_hCamera, CAMERA_MEDIA_TYPE_MONO8);
	}

	strcpy_s(g_CameraName, sCameraList[0].acFriendlyName);

	CameraCreateSettingPage(m_hCamera, NULL,
		 g_CameraName, NULL, NULL, 0);//"通知SDK内部建该相机的属性页面";


	m_hDispThread = (HANDLE)_beginthreadex(NULL, 0, &uiDisplayThread, (PVOID)this, 0, &m_threadID);

	CameraPlay(m_hCamera);

	CameraShowSettingPage(m_hCamera, TRUE);//TRUE显示相机配置界面。FALSE则隐藏。
}
