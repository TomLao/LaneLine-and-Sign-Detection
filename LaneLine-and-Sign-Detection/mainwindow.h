#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDir>
#include <QFileDevice>
#include <QLabel>
#include <QPainter>
#include <QTimer>
#include <QDebug>
#include <QFileDialog>
#include <QCloseEvent>
#include <QMessageBox>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;
using namespace std;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


protected:
    Mat getframe(Mat image);
    int MyRunner();
    Mat start(Mat frame);


private slots:
    void on_pushButton_open_clicked();
    void on_pushButton_start_clicked();
    void on_pushButton_pause_clicked();
    void on_pushButton_stop_clicked();


private:
    Ui::MainWindow *ui;
//    VideoCapture videoCapture;
    Mat srcFrame;
    string videoPath;
    bool isOpen;                //是否选择并打开页面
    bool isStart;               //是否开始检测了
    bool isPause;               //是否暂停检测
    bool isStop;                //是否结束检测
    bool isLaneLine;            //是否检测到车道线
    bool isSign;                //是否检测到标志
    QTimer *timer;
    QImage qimg;


    void closeEvent(QCloseEvent *enent);
};


#endif // MAINWINDOW_H
