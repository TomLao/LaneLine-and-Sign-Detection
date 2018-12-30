#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include "lanedetector.h"
#include <iostream>
using namespace std;
using namespace cv;



int main() {

    // The input argument is the location of the video
    cv::VideoCapture cap("F:\\QT_code\\LiuWenguo_homework\\CourseDesign\\loadVideo3.mp4");
    if (!cap.isOpened())
        return -1;

    LaneDetector lanedetector;  // 创建类对象
    Mat frame;
    Mat img_denoise;
    Mat img_edges;
    Mat img_mask;
    Mat img_lines;
    vector<cv::Vec4i> lines;
    vector<std::vector<cv::Vec4i> > left_right_lines;
    vector<cv::Point> lane;
    string turn;
    int flag_plot = -1;

    // 主要算法开始。遍历视频的每一帧
    while (true) {
        // Capture frame
        if (!cap.read(frame))
            break;
        if(waitKey(1)==27)
            break;

        //图像缩小
        resize(frame,frame,Size(576,324),0,0,INTER_LINEAR);

        // 使用高斯滤波器去噪图像
        img_denoise = lanedetector.deNoise(frame);
        imshow("高斯滤波器去噪图像",frame);

        // 检测图像中的边缘
        img_edges = lanedetector.edgeDetector(img_denoise);
        imshow("检测图像中的边缘",img_edges);

        // 对图像进行掩码，只得到ROI
        img_mask = lanedetector.mask(img_edges);
        imshow("对图像进行掩码，只得到ROI",img_mask);

        // 获取裁剪图像中的霍夫线
        lines = lanedetector.houghLines(img_mask);
    //    imshow("裁剪图像中的霍夫线",lines);

        if (!lines.empty())
        {
            // 把线分成左行和右行
            left_right_lines = lanedetector.lineSeparation(lines, img_edges);

            // 应用回归法，使每条泳道的每一侧只能得到一条直线
            lane = lanedetector.regression(left_right_lines, frame);

            // 通过确定直线的消失点来预测转弯
            turn = lanedetector.predictTurn();

            // 情节车道检测
            flag_plot = lanedetector.plotLane(frame, lane, turn);

            cv::waitKey(1);
        }
        else {
            flag_plot = -1;
        }
    }

    return flag_plot;
}

