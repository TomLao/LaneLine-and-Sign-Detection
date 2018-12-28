//#include "mainwindow.h"

#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <vector>

using namespace cv;
using namespace std;

//转为hls颜色空间
Mat convertHls(Mat image) {
    Mat converted;
    cvtColor(image, converted, CV_RGB2HLS);
    return converted;
}

//根据hls颜色空间选取白黄车道线
Mat selectWhiteYellow(Mat image) {
    Mat converted = convertHls(image);
    Mat mask, maskWhite, maskYellow;
    //取白色和黄色mask
    inRange(converted, Scalar(0, 200, 0), Scalar(255, 255, 255), maskWhite);
    inRange(converted, Scalar(10, 0, 100), Scalar(40, 255, 255), maskYellow);
    //合并
    bitwise_or(maskWhite, maskYellow, mask);
    bitwise_and(image, image, mask);
    return image;
}

//灰度化
Mat convertGrayScale(Mat image) {
    Mat converted;
    cvtColor(image, converted, CV_RGB2GRAY);
    return converted;
}

//高斯模糊
Mat smoothing(Mat image, int kernelSize) {
    Mat smoothed;
    GaussianBlur(image, smoothed, Size(kernelSize, kernelSize), 0);
    return smoothed;
}

//canny边缘检测
Mat detectEdge(Mat image, int lowThreshold, int highThreshold) {
    Mat edge;
    Canny(image, edge, lowThreshold, highThreshold);
    return edge;
}

//选取车道线，区域过滤，去除噪声
Mat selectRegion(Mat image) {
    float rows = image.rows, cols = image.cols;
    Point rootPoints[1][4];
    rootPoints[0][0] = Point(cols * 0.1, rows * 0.95);
    rootPoints[0][1] = Point(cols * 0.4, rows * 0.6);
    rootPoints[0][2] = Point(cols * 0.6, rows * 0.6);
    rootPoints[0][3] = Point(cols * 0.9, rows * 0.95);
    const Point *ppt[1] = {rootPoints[0]};
    Mat mask = image.clone();
    int npt[] = {255};
    fillPoly(mask, ppt, npt, 1, Scalar(255, 255, 255));
    return mask;
}

//Hough线变换
vector<Vec4i> houghLine(Mat image) {
    vector<Vec4i> lines;
    HoughLinesP(image, lines, 1, CV_PI / 180, 20, 20, 300);
    return lines;
}

//在原图上画线
Mat drawLines(Mat image, vector<Vec4i> lines, Scalar color, int thickness, bool makeCopy) {
    Mat copy;
    if (makeCopy) {
        image.copyTo(copy);
    }
    for (size_t i = 0; i < lines.size(); i++) {
        line(copy, Point(lines[i][0], lines[i][1]), Point(lines[i][2], lines[i][3]), color, thickness);
    }
    return copy;
}

//主控函数
Mat start(Mat frame) {
    int gaussianKernelSize = 15;
    int lowThreshold = 50;
    int highThreshold = 150;

//    Mat converted = convertHls(frame);
//    imshow("converted", converted);
//    Mat whiteYellowLines = selectWhiteYellow(converted);
//    imshow("whiteYellowLines", whiteYellowLines);
//    Mat gray = convertGrayScale(whiteYellowLines);
//    imshow("gray", gray);
//    Mat smoothed = smoothing(gray, gaussianKernelSize);
//    imshow("smoothed", smoothed);
//    Mat edge = detectEdge(smoothed, lowThreshold, highThreshold);
//    imshow("edge", edge);
//    Mat roi = selectRegion(edge);
//    imshow("roi", roi);
//    vector<Vec4i> lines = houghLine(roi);
//    Mat lineImage = drawLines(roi, lines, Scalar(255, 0, 0), 3, true);
//    imshow("lineImage", lineImage);
//    return lineImage;

    Mat res;
    cvtColor(frame, res, COLOR_BGR2HLS);

    Mat mask, maskWhite, maskYellow;

    //取白色和黄色mask
    inRange(res, Scalar(0, 200, 0), Scalar(255, 255, 255), maskWhite);
    inRange(res, Scalar(10, 0, 100), Scalar(40, 255, 255), maskYellow);
    //合并
    bitwise_or(maskWhite, maskYellow, mask);
    bitwise_and(frame, frame, res, mask);

    cvtColor(res, res, CV_RGB2GRAY);

    GaussianBlur(res, res, Size(gaussianKernelSize, gaussianKernelSize), 0);

    Canny(res, res, lowThreshold, highThreshold);

    int rows = frame.rows, cols = frame.cols;
    Point rootPoints[1][4];
    cout<<rows<<"  "<<cols<<endl;
    rootPoints[0][0] = Point(cols * 0.1, rows * 0.95);
    rootPoints[0][1] = Point(cols * 0.4, rows * 0.6);
    rootPoints[0][2] = Point(cols * 0.6, rows * 0.6);
    rootPoints[0][3] = Point(cols * 0.9, rows * 0.95);
    const Point *ppt[1] = {rootPoints[0]};
    int npt[] = {50};
    mask = Mat::zeros(res.size(), CV_8UC1); //CV_8UC1 新建单颜色通道
    fillPoly(mask, ppt, npt, 1, 255, 8);

    bitwise_and(res, mask, res);
    imshow("res", res);
//    waitKey(0);
//    return res;
    vector<Vec4i> lines;
    HoughLinesP(res, lines, 1, CV_PI / 180, 20, 20, 300);
    Mat copy = frame.clone();
    for( size_t i = 0; i < lines.size(); i++ )
    {
        Vec4i l = lines[i];
        line(copy , Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0,0,255), 3, CV_AA);
    }


//    imshow("copy", copy);
//    waitKey(0);
    return copy;

}

int main() {
    String path = "FinalProject/test_video/solidYellowLeft.mp4";
    VideoCapture videoCapture(path);
    double FPS = videoCapture.get(CAP_PROP_FPS);
    Mat frame;
    while (1) {
        videoCapture >> frame;
        if (frame.empty())
            break;
        Mat res = start(frame);
        imshow("detect", res);
        if (waitKey(1000.0 / FPS) == 27)
            break;
    }
//    Mat image = imread(path);
//    Mat res = start(image);
//    imshow("detect", res);
//    waitKey(0);
}