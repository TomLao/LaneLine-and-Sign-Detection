#ifndef LANEDETECTOR_H
#define LANEDETECTOR_H
#include "iostream";
using namespace std;
using namespace cv;
class LaneDetector
{
private:
    double img_size;
    double img_center;
    bool left_flag = false;  // 告诉我们是否检测到车道的左边界
    bool right_flag = false;  // 告诉我们是否检测到车道的右边界
    Point right_b;  // 车道边界线方程的两个成员:
    double right_m;  // y = m*x + b
    Point left_b;  //
    double left_m;  //

public:
    Mat deNoise(Mat inputImage);  // 对输入图像应用高斯模糊
    Mat edgeDetector(Mat img_noise);  // 过滤图像，只获取边缘
    Mat mask(Mat img_edges);  // 掩盖边缘图像，只关心ROI
    vector<Vec4i> houghLines(Mat img_mask);  // 检测蒙面边缘图像中的霍夫线
    vector<vector<Vec4i> > lineSeparation(vector<Vec4i> lines, Mat img_edges);  // Sprt通过直线的斜率检测出左右直线
    vector<Point> regression(vector<vector<Vec4i> > left_right_lines, Mat inputImage);  //每条车道只能排一条线
    string predictTurn();  //通过计算消失点的位置来确定车道是否在转弯
    int plotLane(Mat inputImage, vector<Point> lane, string turn);  //绘制合成车道并在框架中进行转弯预测。
};
#endif // LANEDETECTOR_H
