/******************************************************
author: XiuRuiwei
co-author: LaoHeze
Date: 2019-1-2

所有构成LaneDetector类的一部分的函数。
类将以RGB图像作为输入并输出相同的RGB图像，但是
带有检测到的车道图和转弯预测。
********************************************************/
#include "lanedetector.h"
using namespace std;


/**
 * @brief LaneDetector::detector qt检测算法运行模块
 * @param frame 结果缩放的原始图像
 * @author HezeLao
 */
void LaneDetector::myDetector(Mat frame){
    Mat img_denoise;
    Mat img_edges;
    Mat img_mask;
    Mat img_lines;
    vector<cv::Vec4i> lines;
    vector<std::vector<cv::Vec4i> > left_right_lines;
    vector<cv::Point> lane;
//    string turn;

//    streetSign(frame);  //交通标志检测及显示

    img_denoise = deNoise(frame);// 使用高斯滤波器去噪图像
    img_edges = edgeDetector(img_denoise);// 检测图像中的边缘
    img_mask = mask(img_edges);// 对图像进行掩码，只得到ROI
    lines = houghLines(img_mask);//霍夫变换

    if (!lines.empty()) {
        left_right_lines = lineSeparation(lines, img_edges);// 把线分成左行和右行
        lane = regression(left_right_lines, frame);// 应用回归法，使每条泳道的每一侧只能得到一条直线
        //turn = predictTurn();// 通过确定直线的消失点来预测转弯
        //plotLane(frame, lane, turn);// 画出车道线
        plotLane(frame, lane);// 画出车道线
    }
}


//运行检测代码块
//void LaneDetector::doDetection(String videoPath){
//    // The input argument is the location of the video
//    cv::VideoCapture cap(videoPath);
//    if (!cap.isOpened())
//        return ;
//    Mat frame;
//    Mat img_denoise;
//    Mat img_edges;
//    Mat img_mask;
//    Mat img_lines;
//    vector<cv::Vec4i> lines;
//    vector<std::vector<cv::Vec4i> > left_right_lines;
//    vector<cv::Point> lane;
//    string turn;
//    int flag_plot = -1;

//    // 主要算法开始。遍历视频的每一帧
//    while (true) {
//        if (!cap.read(frame))
//            break;
//        if(waitKey(1)==27)
//            break;

//        //图像缩小
//        resize(frame,frame,Size(576,324),0,0,INTER_LINEAR);

//        //交通标志检测及显示
//        streetSign(frame);

//        // 使用高斯滤波器去噪图像
//        img_denoise = deNoise(frame);
//        //        imshow("高斯滤波器去噪图像",frame);

//        // 检测图像中的边缘
//        img_edges = edgeDetector(img_denoise);
//        //        imshow("检测图像中的边缘",img_edges);

//        // 对图像进行掩码，只得到ROI
//        img_mask = mask(img_edges);
//        //        imshow("对图像进行掩码，只得到ROI",img_mask);

//        // 获取裁剪图像中的霍夫线
//        lines = houghLines(img_mask);
//        //   imshow("裁剪图像中的霍夫线",lines);

//        if (!lines.empty())
//        {
//            // 把线分成左行和右行
//            left_right_lines = lineSeparation(lines, img_edges);

//            // 应用回归法，使每条泳道的每一侧只能得到一条直线
//            lane = regression(left_right_lines, frame);

//            // 通过确定直线的消失点来预测转弯
//            turn = predictTurn();

//            // 情节车道检测
//            flag_plot = plotLane(frame, lane, turn);
//        }
//        else {
//            flag_plot = -1;
//        }
//    }

//    return ;
//}


/**
 * @brief LaneDetector::streetSign 运用HSV颜色空间进行路牌检测
 * @param img_original
 * @author XieRuiwei LaoHeze
 */
Mat LaneDetector::streetSign(Mat img_original){
    Mat hsv, roi;
    cvtColor(img_original, hsv, COLOR_RGB2HSV); //直接转换#HSV颜色空间

    //颜色筛选
    Mat mask1;
    inRange(hsv, Scalar(155,60,60), Scalar(180,255,255), mask1);    //红色
    Mat mask2;
    inRange(hsv, Scalar(97,80,45), Scalar(124,255,255), mask2);     //蓝色
    Mat mask = mask1 + mask2;
    GaussianBlur(mask, mask,Size(5, 5), 3, 3);              //高斯模糊

    Mat binary;
    threshold(mask, binary,127, 255, CV_THRESH_BINARY);     //二值化

    //定义核，形态学闭运算
    Mat closed, kernel = getStructuringElement(MORPH_RECT,Size(21,7));
    morphologyEx(binary,closed, MORPH_CLOSE, kernel);

    Mat img_erode,img_dilate;
    erode(closed,img_erode, NULL, Point(-1, -1), 4);        //腐蚀
    dilate(img_erode,img_dilate, NULL, Point(-1, -1), 4);   //膨胀

    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;        //分层
    findContours(img_dilate, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));   //寻找轮廓

    for (int i = 0; i < contours.size(); i++){
        double area = contourArea(Mat(contours[i]),false);
        if(area > 0.055*576*324 || area <0.0012*576*324)
            continue;

        //变换为旋转矩阵
        RotatedRect rect=minAreaRect(Mat(contours[i]));     //最小化矩形区域
        //定义一个存储以上四个点的坐标的变量
        Point2f fourPoint2f[4];
        //将rectPoint变量中存储的坐标值放到 fourPoint的数组中
        rect.points(fourPoint2f);
        double x,y,width,height;
        x = y = width = height = 0;
        x = fourPoint2f[1].x;
        y = fourPoint2f[1].y;
        width = fabs(fourPoint2f[0].x - fourPoint2f[3].x);
        height = fabs(fourPoint2f[0].y - fourPoint2f[1].y);

        if(width/height>1.2 && width/height<2.5){           //根据矩形的长宽比  圈出标志牌
            //根据得到的四个点的坐标  绘制矩形
            Scalar color = (0, 0, 255);                     //蓝色线画轮廓
            for (int i = 0; i < 3; i++) {
                line(img_original, fourPoint2f[i], fourPoint2f[i + 1], color, 3);
            }
            line(img_original, fourPoint2f[3], fourPoint2f[0], color, 3);

            //显示裁剪后的图片
            if(x>50 && y>50 && width >50 && height >20){
                roi.release();
                roi = img_original(Rect(x,y,width,height));
                imshow("裁剪后的图片",roi);
            }
        }
    }
    return roi;
}


/**
 *@brief 对输入图像进行高斯滤波去噪
 * @param inputImage是一个视频的帧，其中
 * @param lane将被检测到
 * @return 模糊和去噪图像
 */
Mat LaneDetector::deNoise(Mat inputImage) {
    Mat output;
    GaussianBlur(inputImage, output, Size(3, 3), 0, 0);
    return output;
}


/**
 * @brief 通过过滤图像检测模糊帧中的所有边缘
 * @param img_noise是之前模糊的帧
 * @return 只有边缘用白色表示的二进制图像
 */
Mat LaneDetector::edgeDetector(Mat img_noise) {
    Mat output;
    Mat kernel;
    Point anchor;

    cvtColor(img_noise, output, COLOR_RGB2GRAY);        //灰度化
    threshold(output, output, 140, 255, THRESH_BINARY); //二值化

    //创建内核 kernel[-1 0 1]，这个内核是基于，Mathworks的车道偏离警告系统
    anchor = Point(-1, -1);
    kernel = Mat(1, 3, CV_32F);
    kernel.at<float>(0, 0) = -1;
    kernel.at<float>(0, 1) = 0;
    kernel.at<float>(0, 2) = 1;

    // 对二值图像进行滤波以获得边缘
    filter2D(output, output, -1, kernel, anchor, 0, BORDER_DEFAULT);
    return output;
}


/**
 * @brief 蒙版边缘图像，对图像进行掩码，只检测到构成车道一部分的边缘
 * @param img_edge是前一个函数的边缘图像
 * @return 二进制图像，只有需要的边缘具有代表性d
 */
Mat LaneDetector::mask(Mat img_edges) {
    Mat output;
    Mat mask = Mat::zeros(img_edges.size(), img_edges.type());
    Point pts[4] = {
        //        //非老师给的视频参数   loadVideo.mp4
        //        Point(80,368),    //左下角
        //        Point(120,280),   //左上角
        //        Point(600,280),
        //        Point(800,368)    //右下角

        //老师给的视频参数，长的   loadVideo1.mp4 2.MP4
        Point(-120,368),    //左下角
        Point(50,250),   //左上角
        Point(290,250),
        Point(450,368)    //右下角

        //        //老师给的视频参数   loadVideo2.mp4 1.MP4
        //        Point(-60,368),    //左下角
        //        Point(80,220),   //左上角
        //        Point(450,220),
        //        Point(600,368)    //右下角

        //        //刘文果给的视频参数   loadVideo3.mp4 4.mp4
        //         Point(-90,368),    //左下角
        //         Point(40,235),   //左上角
        //         Point(350,235),
        //         Point(650,368)    //右下角
    };

    fillConvexPoly(mask, pts, 4, Scalar(255, 0, 0));    //掩模填充
    bitwise_and(img_edges, mask, output);   //取并集

    return output;
}


/**
 * @brief 获取所有蒙面图像中的线段，这些线段将成为车道边界的一部分
 * @param img_mask是前一个函数的屏蔽二进制映像
 * @return 包含图像中所有检测到的线的向量
 */
vector<Vec4i> LaneDetector::houghLines(Mat img_mask) {
    vector<Vec4i> line;
    HoughLinesP(img_mask, line, 1, CV_PI / 180, 20, 20, 30);    // 和是通过反复试验得到的
    return line;
}


// 对左右行进行排序
/**
 * @brief 按坡度对所有检测到的霍夫线进行排序。
 * @brief 这些线可以分为左右两种
 * @brief 关于他们的坡度符号和大致位置
 * @param 线是包含所有检测到的线的向量
 * @param img_edge用于确定图像中心
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

// 左右线的回归420/5000
/**
 * @brief 回归采用所有分类线段的起始点和结束点，用最小二乘法拟合出一条新的线段。
 * @brief 左右两边都做了。
 * @param left_right_lines是lineSeparation函数的输出
 * @param inputImage用于选择行结束的位置
 * @return 输出包含两个车道边界线的起始点和终点
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
    int fin_y = 250;

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

/**
 * @brief 预测, 预测车道是左转、右转还是直走
 * @brief 通过查看消失点相对于图像中心的位置来完成
 * @return 字符串，表示是否有左或右转弯或道路是否笔直
 */
//string LaneDetector::predictTurn() {
//    string output;
//    double vanish_x;
//    double thr_vp = 10;

//    // 消失点是两个车道边界线相交的点
//    vanish_x = static_cast<double>(((right_m*right_b.x) - (left_m*left_b.x) - right_b.y + left_b.y) / (right_m - left_m));

//    // 消失点的位置决定了道路转弯的位置
//    if (vanish_x < (img_center - thr_vp))
//        output = "Left Turn";
//    else if (vanish_x >(img_center + thr_vp))
//        output = "Right Turn";
//    else if (vanish_x >= (img_center - thr_vp) && vanish_x <= (img_center + thr_vp))
//        output = "Straight";

//    return output;
//}

//绘制结果
/**
 * @brief该函数绘制车道两侧、转弯预测信息和覆盖车道边界内区域的透明多边形
 * @param inputImage是原始捕获帧
 * @param lane是包含两条直线信息的向量
 * @param turn是包含转弯信息的输出字符串
 * @return函数返回0
 */
void LaneDetector::plotLane(Mat inputImage, vector<Point> lane) {
    vector<Point> poly_points;
    Mat output;

    // 创建透明多边形，以便更好地显示车道
    inputImage.copyTo(output);
    poly_points.push_back(lane[2]);
    poly_points.push_back(lane[0]);
    poly_points.push_back(lane[1]);
    poly_points.push_back(lane[3]);
    fillConvexPoly(output, poly_points, Scalar(255, 0, 0), CV_AA, 0);
    addWeighted(output, 0.3, inputImage, 1.0 - 0.3, 0, inputImage);

    // 绘制车道边界的两条线
    //    line(inputImage, lane[0], lane[1], Scalar(0, 0, 255), 5, CV_AA);
    //    line(inputImage, lane[2], lane[3], Scalar(0, 0, 255), 5, CV_AA);

    // 绘制转弯信息
    //    putText(inputImage, turn, Point(50, 90), FONT_HERSHEY_COMPLEX_SMALL, 3, cvScalar(0, 255, 0), 1, CV_AA);

    // 显示最终输出图像
    //    namedWindow("Lane", CV_WINDOW_AUTOSIZE);
    //    imshow("Lane", inputImage);
    //    return 0;
}
