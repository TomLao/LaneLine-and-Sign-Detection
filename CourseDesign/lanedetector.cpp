/***
所有构成LaneDetector类的一部分的函数。
类将以RGB图像作为输入并输出相同的RGB图像，但是
带有检测到的车道图和转弯预测。
*/
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include "lanedetector.h"

// 图像模糊
/**
*@brief对输入图像进行高斯滤波去噪
*@param inputImage是一个视频的帧，其中
*@param lane将被检测到
*@return模糊和去噪图像
*/
Mat LaneDetector::deNoise(Mat inputImage) {
    Mat output;
    GaussianBlur(inputImage, output, Size(3, 3), 0, 0);
    return output;
}

// 边缘检测
/**
*@brief通过过滤图像检测模糊帧中的所有边缘
*@param img_noise是之前模糊的帧
*@return只有边缘用白色表示的二进制图像
*/
Mat LaneDetector::edgeDetector(Mat img_noise) {
    Mat output;
    Mat kernel;
    Point anchor;

    // 将图像从RGB转换为灰色
    cvtColor(img_noise, output, COLOR_RGB2GRAY);
    // Binarize灰度图像
    threshold(output, output, 140, 255, THRESH_BINARY);

    //创建内核 kernel[-1 0 1]
    //这个内核是基于
    // Mathworks的车道偏离警告系统
    anchor = Point(-1, -1);
    kernel = Mat(1, 3, CV_32F);
    kernel.at<float>(0, 0) = -1;
    kernel.at<float>(0, 1) = 0;
    kernel.at<float>(0, 2) = 1;

    // 对二值图像进行滤波以获得边缘
    filter2D(output, output, -1, kernel, anchor, 0, BORDER_DEFAULT);
//    imshow("output", output);
    return output;
}

// 蒙版边缘图像
/**
*@brief对图像进行掩码，只检测到构成车道一部分的边缘
*@param img_edge是前一个函数的边缘图像
*@return二进制图像，只有需要的边缘具有代表性d
*/
Mat LaneDetector::mask(Mat img_edges) {
    Mat output;
    Mat mask = Mat::zeros(img_edges.size(), img_edges.type());
    Point pts[4] = {
        Point(0,368),    //左下角
        Point(100,280),   //左上角
        Point(380,280),
        Point(500,368)    //右下角
    };

    // 创建一个二元多边形掩模
    fillConvexPoly(mask, pts, 4, Scalar(255, 0, 0));
    // 将边缘图像和蒙版相乘得到输出
    bitwise_and(img_edges, mask, output);

    imshow("mask",mask);
    return output;
}

// HOUGH LINES
/**
*@brief获取所有蒙面图像中的线段，这些线段将成为车道边界的一部分
*@param img_mask是前一个函数的屏蔽二进制映像
*@return包含图像中所有检测到的线的向量
*/
vector<Vec4i> LaneDetector::houghLines(Mat img_mask) {
    vector<Vec4i> line;
    // 和是通过反复试验得到的
    HoughLinesP(img_mask, line, 1, CV_PI / 180, 20, 20, 30);
    return line;
}

// 对左右行进行排序
/**
*@brief按坡度对所有检测到的霍夫线进行排序。
*@brief这些线可以分为左右两种
*@brief关于他们的坡度符号和大致位置
*@param线是包含所有检测到的线的向量
*@param img_edge用于确定图像中心
输出是一个向量(2)，它包含所有分类行
*/
vector<vector<Vec4i> > LaneDetector::lineSeparation(vector<Vec4i> lines, Mat img_edges) {
    vector<vector<Vec4i> > output(2);
    size_t j = 0;
    Point ini;
    Point fini;
    double slope_thresh = 0.3;
    vector<double> slopes;
    vector<Vec4i> selected_lines;
    vector<Vec4i> right_lines, left_lines;

    // 计算所有被测直线的斜率
    for (auto i : lines) {
        ini = Point(i[0], i[1]);
        fini = Point(i[2], i[3]);

        // 基本代数:斜率  slope = (y1 - y0)/(x1 - x0)
        double slope = (static_cast<double>(fini.y) - static_cast<double>(ini.y)) / (static_cast<double>(fini.x) - static_cast<double>(ini.x) + 0.00001);

        //如果坡度太水平，就放弃这条线
        //如果没有，保存它们和它们各自的斜率
        if (abs(slope) > slope_thresh) {
            slopes.push_back(slope);
            selected_lines.push_back(i);
        }
    }

    // 把线分成左右两行
    img_center = static_cast<double>((img_edges.cols / 2));
    while (j < selected_lines.size()) {
        ini = Point(selected_lines[j][0], selected_lines[j][1]);
        fini = Point(selected_lines[j][2], selected_lines[j][3]);

        // 将线分为左边或右边的条件
        if (slopes[j] > 0 && fini.x > img_center && ini.x > img_center) {
            right_lines.push_back(selected_lines[j]);
            right_flag = true;
        }
        else if (slopes[j] < 0 && fini.x < img_center && ini.x < img_center) {
            left_lines.push_back(selected_lines[j]);
            left_flag = true;
        }
        j++;
    }

    output[0] = right_lines;
    output[1] = left_lines;

    return output;
}

// 左右线的回归
/**

420/5000
*@brief回归采用所有分类线段的起始点和结束点，用最小二乘法拟合出一条新的线段。
*@brief左右两边都做了。
*@param left_right_lines是lineSeparation函数的输出
*@param inputImage用于选择行结束的位置
*@return输出包含两个车道边界线的起始点和终点
*/
vector<Point> LaneDetector::regression(vector<vector<Vec4i> > left_right_lines, Mat inputImage) {
    vector<Point> output(4);
    Point ini;
    Point fini;
    Point ini2;
    Point fini2;
    Vec4d right_line;
    Vec4d left_line;
    vector<Point> right_pts;
    vector<Point> left_pts;

    // 如果检测到右行，请使用该行的所有初始化和最终点匹配该行
    if (right_flag == true) {
        for (auto i : left_right_lines[0]) {
            ini = Point(i[0], i[1]);
            fini = Point(i[2], i[3]);

            right_pts.push_back(ini);
            right_pts.push_back(fini);
        }

        if (right_pts.size() > 0) {
            // 这里形成了一条直线
            fitLine(right_pts, right_line, CV_DIST_L2, 0, 0.01, 0.01);
            right_m = right_line[1] / right_line[0];
            right_b = Point(right_line[2], right_line[3]);
        }
    }

    // 如果检测到左行，使用该行的所有init和final点拟合一行
    if (left_flag == true) {
        for (auto j : left_right_lines[1]) {
            ini2 = Point(j[0], j[1]);
            fini2 = Point(j[2], j[3]);

            left_pts.push_back(ini2);
            left_pts.push_back(fini2);
        }
        if (left_pts.size() > 0) {
            // 左线在这里形成
            fitLine(left_pts, left_line, CV_DIST_L2, 0, 0.01, 0.01);
            left_m = left_line[1] / left_line[0];
            left_b = Point(left_line[2], left_line[3]);
        }
    }

    // 首先求出了直线的斜率和偏置点，应用直线方程求出了直线点
    int ini_y = inputImage.rows;
    int fin_y = 280;

    double right_ini_x = ((ini_y - right_b.y) / right_m) + right_b.x;
    double right_fin_x = ((fin_y - right_b.y) / right_m) + right_b.x;

    double left_ini_x = ((ini_y - left_b.y) / left_m) + left_b.x;
    double left_fin_x = ((fin_y - left_b.y) / left_m) + left_b.x;

    output[0] = Point(right_ini_x, ini_y);
    output[1] = Point(right_fin_x, fin_y);
    output[2] = Point(left_ini_x, ini_y);
    output[3] = Point(left_fin_x, fin_y);

    return output;
}

// 预测
/**
*@brief预测车道是左转、右转还是直走
*@brief通过查看消失点相对于图像中心的位置来完成
*@return字符串，表示是否有左或右转弯或道路是否笔直
*/
string LaneDetector::predictTurn() {
    string output;
    double vanish_x;
    double thr_vp = 10;

    // 消失点是两个车道边界线相交的点
    vanish_x = static_cast<double>(((right_m*right_b.x) - (left_m*left_b.x) - right_b.y + left_b.y) / (right_m - left_m));

    // 消失点的位置决定了道路转弯的位置
    if (vanish_x < (img_center - thr_vp))
        output = "Left Turn";
    else if (vanish_x >(img_center + thr_vp))
        output = "Right Turn";
    else if (vanish_x >= (img_center - thr_vp) && vanish_x <= (img_center + thr_vp))
        output = "Straight";

    return output;
}

//绘制结果
/**
*@brief该函数绘制车道两侧、转弯预测信息和覆盖车道边界内区域的透明多边形
*@param inputImage是原始捕获帧
*@param lane是包含两条直线信息的向量
*@param turn是包含转弯信息的输出字符串
*@return函数返回0
*/
int LaneDetector::plotLane(Mat inputImage, vector<Point> lane, string turn) {
    vector<Point> poly_points;
    Mat output;

    // 创建透明多边形，以便更好地显示车道
    inputImage.copyTo(output);
    poly_points.push_back(lane[2]);
    poly_points.push_back(lane[0]);
    poly_points.push_back(lane[1]);
    poly_points.push_back(lane[3]);
    fillConvexPoly(output, poly_points, Scalar(0, 0, 255), CV_AA, 0);
    addWeighted(output, 0.3, inputImage, 1.0 - 0.3, 0, inputImage);

    // 绘制车道边界的两条线
    line(inputImage, lane[0], lane[1], Scalar(0, 255, 255), 5, CV_AA);
    line(inputImage, lane[2], lane[3], Scalar(0, 255, 255), 5, CV_AA);

    // 绘制转弯信息
    putText(inputImage, turn, Point(50, 90), FONT_HERSHEY_COMPLEX_SMALL, 3, cvScalar(0, 255, 0), 1, CV_AA);

    // 显示最终输出图像
    namedWindow("Lane", CV_WINDOW_AUTOSIZE);
    imshow("Lane", inputImage);
    return 0;
}
