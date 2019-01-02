#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include "lanedetector.h"
#include <iostream>
using namespace std;
using namespace cv;



int main() {

       LaneDetector lanedetector;  // 创建类对象
       lanedetector.doDetection("F:\\QT_code\\LiuWenguo_homework\\CourseDesign\\loadVideo3.mp4");
}

