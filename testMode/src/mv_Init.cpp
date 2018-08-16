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
			//����õ�ԭʼ����ת����RGB��ʽ�����ݣ�ͬʱ����ISPģ�飬��ͼ����н��룬������������ɫУ���ȴ���
			//�ҹ�˾�󲿷��ͺŵ������ԭʼ���ݶ���Bayer��ʽ��
			status = CameraImageProcess(pThis->m_hCamera, pbyBuffer, pThis->m_pFrameBuffer, &sFrameInfo);//����ģʽ

																						 //�ֱ��ʸı��ˣ���ˢ�±���
			if (IMAGE_COLS != sFrameInfo.iWidth || IMAGE_ROWS != sFrameInfo.iHeight)
			{
				cout << "cols:" << sFrameInfo.iWidth << "rows" << sFrameInfo.iHeight << endl;
				exit(0);
			}
			else
			{
				// �������Ĳ�����Ϣ
				memcpy(&pThis->m_sFrInfo, &sFrameInfo, sizeof(tSdkFrameHead));
			}

			if (status == CAMERA_STATUS_SUCCESS)
			{
				//����SDK��װ�õ���ʾ�ӿ�����ʾͼ��,��Ҳ���Խ�m_pFrameBuffer�е�RGB����ͨ��������ʽ��ʾ������directX,OpengGL,�ȷ�ʽ��
				//CameraImageOverlay(hCamera, m_pFrameBuffer, &sFrameInfo);

				cv::Mat matImage(
					cvSize(sFrameInfo.iWidth, sFrameInfo.iHeight),
					sFrameInfo.uiMediaType == CAMERA_MEDIA_TYPE_MONO8 ? CV_8UC1 : CV_8UC3,
					pThis->m_pFrameBuffer
				);
				matImage.copyTo(pThis->srcImage);

				SetEvent(pThis->g_hEventBufferFull);
			}

			//�ڳɹ�����CameraGetImageBuffer�󣬱������CameraReleaseImageBuffer���ͷŻ�õ�buffer��
			//�����ٴε���CameraGetImageBufferʱ�����򽫱�����֪�������߳��е���CameraReleaseImageBuffer���ͷ���buffer
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

	 //���������Զ���λ�¼���һ����ʾ�������Ƿ�Ϊ�գ���һ����ʾ�������Ƿ��Ѿ�����
	 g_hEventBufferFull = CreateEvent(NULL, FALSE, FALSE, NULL);

	 //ö���豸������豸�б�
	 iCameraNums = CAMERA_NUM;//����CameraEnumerateDeviceǰ��������iCameraNums = 10����ʾ���ֻ��ȡ10���豸�������Ҫö�ٸ�����豸�������sCameraList����Ĵ�С��iCameraNums��ֵ

	 if (CameraEnumerateDevice(sCameraList, &iCameraNums) != CAMERA_STATUS_SUCCESS || iCameraNums == 0)
	 {
		 printf("No camera was found!");
		 return FALSE;
	 }


	// ��ʼ�����
	if ((status = CameraInit(&sCameraList[0], -1, -1, &m_hCamera)) != CAMERA_STATUS_SUCCESS)
	{
		char msg[128];
		sprintf_s(msg, "Failed to init the camera! Error code is %d", status);
		printf(msg);
		printf(CameraGetErrorString(status));
		return FALSE;
	}

	//"��ø��������������"
	CameraGetCapability(m_hCamera, &sCameraInfo);

	m_pFrameBuffer = (BYTE *)CameraAlignMalloc(sCameraInfo.sResolutionRange.iWidthMax*sCameraInfo.sResolutionRange.iWidthMax * 3, 16);


	if (sCameraInfo.sIspCapacity.bMonoSensor)
	{
		CameraSetIspOutFormat(m_hCamera, CAMERA_MEDIA_TYPE_MONO8);
	}

	strcpy_s(g_CameraName, sCameraList[0].acFriendlyName);

	CameraCreateSettingPage(m_hCamera, NULL,
		 g_CameraName, NULL, NULL, 0);//"֪ͨSDK�ڲ��������������ҳ��";


	m_hDispThread = (HANDLE)_beginthreadex(NULL, 0, &uiDisplayThread, (PVOID)this, 0, &m_threadID);

	CameraPlay(m_hCamera);

	CameraShowSettingPage(m_hCamera, TRUE);//TRUE��ʾ������ý��档FALSE�����ء�
}
