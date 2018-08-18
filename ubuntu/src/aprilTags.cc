
#include "opencv2/opencv.hpp"
#include "apriltag.h"
#include "tag36h11.h"
#include "tag36h10.h"
#include "tag36artoolkit.h"
#include "tag25h9.h"
#include "tag25h7.h"
#include "common/getopt.h"
#include "aprilTags.h"

using namespace cv;


AprilTags::AprilTags(int argc,char *argv[])
{
    getopt = getopt_create();

    getopt_add_bool(getopt, 'h', "help", 0, "Show this help");
    // s是否输出调试信息
    getopt_add_bool(getopt, 'd', "debug", 0, "Enable debugging output (slow)");
    getopt_add_bool(getopt, 'q', "quiet", 0, "Reduce output");
    getopt_add_string(getopt, 'f', "family", "tag36h11", "Tag family to use");
    getopt_add_int(getopt, '\0', "border", "1", "Set tag family border size");
    getopt_add_int(getopt, 't', "threads", "4", "Use this many CPU threads");
    // 对图片进行缩小
    getopt_add_double(getopt, 'x', "decimate", "1.0", "Decimate input image by this factor");
    // 输入为高斯模型方差
    getopt_add_double(getopt, 'b', "blur", "0.0", "Apply low-pass blur to input");
    getopt_add_bool(getopt, '0', "refine-edges", 1, "Spend more time trying to align edges of tags");
    getopt_add_bool(getopt, '1', "refine-decode", 0, "Spend more time trying to decode tags");
    getopt_add_bool(getopt, '2', "refine-pose", 0, "Spend more time trying to precisely localize tags");

    if (!getopt_parse(getopt, argc, argv, 1) ||
            getopt_get_bool(getopt, "help")) {
        printf("Usage: %s [options]\n", argv[0]);
        getopt_do_usage(getopt);
        exit(0);
    }

    tf = NULL;

    famname = getopt_get_string(getopt, "family");
    // 配置aprilTag的相关参数
    if (!strcmp(famname, "tag36h11"))
        tf = tag36h11_create();
    else if (!strcmp(famname, "tag36h10"))
        tf = tag36h10_create();
    else if (!strcmp(famname, "tag36artoolkit"))
        tf = tag36artoolkit_create();
    else if (!strcmp(famname, "tag25h9"))
        tf = tag25h9_create();
    else if (!strcmp(famname, "tag25h7"))
        tf = tag25h7_create();
    else {
        printf("Unrecognized tag family name. Use e.g. \"tag36h11\".\n");
        exit(-1);
    }
    tf->black_border = getopt_get_int(getopt, "border");


    td = apriltag_detector_create();

    apriltag_detector_add_family(td, tf);
    td->quad_decimate = getopt_get_double(getopt, "decimate");
    td->quad_sigma = getopt_get_double(getopt, "blur");
    td->nthreads = getopt_get_int(getopt, "threads");
    td->debug = getopt_get_bool(getopt, "debug");
    td->refine_edges = getopt_get_bool(getopt, "refine-edges");
    td->refine_decode = getopt_get_bool(getopt, "refine-decode");
    td->refine_pose = getopt_get_bool(getopt, "refine-pose");
}

AprilTags::~AprilTags()
{
    apriltag_detector_destroy(td);
    if (!strcmp(famname, "tag36h11"))
        tag36h11_destroy(tf);
    else if (!strcmp(famname, "tag36h10"))
        tag36h10_destroy(tf);
    else if (!strcmp(famname, "tag36artoolkit"))
        tag36artoolkit_destroy(tf);
    else if (!strcmp(famname, "tag25h9"))
        tag25h9_destroy(tf);
    else if (!strcmp(famname, "tag25h7"))
        tag25h7_destroy(tf);
    getopt_destroy(getopt);
}

void AprilTags::targetDetect(image_u8_t *im)
{
    image_u8_t tempIm = *im;
    detections = apriltag_detector_detect(td, &tempIm);
    // cout << zarray_size(detections) << " tags detected" << endl;
}

void AprilTags::drawLine()
{
    // Draw detection outlines
    for (int i = 0; i < zarray_size(detections); i++) {
         apriltag_detection_t *det = NULL;
         targetPoints.clear();
         zarray_get(detections, i, &det);

        line(srcImg, Point(det->p[0][0], det->p[0][1]),
                    Point(det->p[1][0], det->p[1][1]),
                    Scalar(0, 0xff, 0), 2);
        line(srcImg, Point(det->p[0][0], det->p[0][1]),
                    Point(det->p[3][0], det->p[3][1]),
                    Scalar(0, 0, 0xff), 2);
        line(srcImg, Point(det->p[1][0], det->p[1][1]),
                    Point(det->p[2][0], det->p[2][1]),
                    Scalar(0xff, 0, 0), 2);
        line(srcImg, Point(det->p[2][0], det->p[2][1]),
                    Point(det->p[3][0], det->p[3][1]),
                    Scalar(0xff, 0, 0), 2);
        stringstream ss;
        ss << det->id;
        String text = ss.str();
        int fontface = FONT_HERSHEY_SCRIPT_SIMPLEX;
        double fontscale = 1.0;
        int baseline;
        Size textsize = getTextSize(text, fontface, fontscale, 2,
                                        &baseline);
        putText(srcImg, text, Point(det->c[0]-textsize.width/2,
                                    det->c[1]+textsize.height/2),
                fontface, fontscale, Scalar(0xff, 0x99, 0), 2);
        
        targetPoints.push_back(Point2f(det->p[3][0],det->p[3][1]));
        targetPoints.push_back(Point2f(det->p[2][0],det->p[2][1]));
        targetPoints.push_back(Point2f(det->p[0][0],det->p[0][1]));
        targetPoints.push_back(Point2f(det->p[1][0],det->p[1][1]));
        // std::cout << Point2f(det->p[3][0],det->p[3][1]) << " ";
        // std::cout << Point2f(det->p[2][0],det->p[2][1]) << " ";
        // std::cout << Point2f(det->p[0][0],det->p[0][1]) << std::endl;
        // std::cout << Point2f(det->p[1][0],det->p[1][1]) << " "; 
        //printf("%d,%d\r\n",det->c[0]-textsize.width/2,det->c[1]+textsize.height/2);
        // printf("%d\r\n",(int)det->c[0]);
        //  std::cout << text <<std::endl;        
    }
     zarray_destroy(detections);

     imshow("Tag Detections", srcImg);
}