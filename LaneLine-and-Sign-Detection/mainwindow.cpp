#include "mainwindow.h"
#include "lanedetector.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QString title = QString::fromLocal8Bit("HezeLao-RuiweiXie-车道线识别和路牌识别");
    setWindowTitle(title);

    //初始化
    videoPath = "";
    isOpen = false;
    isStart = false;
    isPause = false;
    isStop = false;
    isLaneLine = true;
    isSign = true;
}

MainWindow::~MainWindow()
{
    delete ui;
}


/**
 * @brief MainWindow::MyRunner 检测控制函数
 * @author HezeLao
 * @date 2018-12-28 23:24
 */
int MainWindow::MyRunner(bool isSign){
    isStart = true;

    VideoCapture videoCapture(videoPath);
    if(!videoCapture.isOpened()){
        QMessageBox::critical(NULL, "critical", QString::fromLocal8Bit("视频打开失败"), QMessageBox::Yes);
        isStart = false;
        isOpen = false;
        return -1;
    }

    //获取帧数，不然会播放速度不对
    int length = videoCapture.get(CAP_PROP_FRAME_COUNT)/videoCapture.get(CAP_PROP_FPS);
    double FPS = videoCapture.get(CAP_PROP_FPS);
    Mat res;
    bool haveSign;
    int countShow=1;
    LaneDetector lanedetector;  // 创建类对象
    while(1){
//        waitKey(1000.0 / FPS);
        waitKey(5);
        if(isPause == true) //暂停检测
            continue;

        videoCapture>>srcFrame;

        if(srcFrame.empty() || isStop == true){
            ui->label_video->clear();   //视频结束或按下结束按钮，清理播放窗口，退出
            ui->label_sign_1->clear();
            ui->label_sign_2->clear();
            videoCapture.release();
            break;
        }

        cv::resize(srcFrame, srcFrame, Size(576, 324), 0, 0, INTER_LINEAR);
        cvtColor(srcFrame, srcFrame, COLOR_BGR2RGB);

        //TODO:从这里出发执行检测算法，分别调用路牌检测和车道线检测
        if(isSign){
            haveSign = lanedetector.streetSign(srcFrame);
            qimg = QImage((const uchar*)(srcFrame.data),srcFrame.cols,srcFrame.rows, QImage::Format_RGB888); //简单地转换一下为Image对象，rgbSwapped是为了显示效果色彩好一些。
            ui->label_video->setPixmap(QPixmap::fromImage(qimg));
            ui->label_video->show();
            if(haveSign){
                qsign = QImage("cut.jpg");  //读取截取的路牌
                if(countShow%50==0){
                    ui->label_sign_1->setPixmap(QPixmap::fromImage(qsign));
                    ui->label_sign_1->show();
                }
                else if(countShow%50==25){
                    ui->label_sign_2->setPixmap(QPixmap::fromImage(qsign));
                    ui->label_sign_2->show();
                }
                countShow++;
            }
        }else{
            lanedetector.myDetector(srcFrame);
            qimg = QImage((const uchar*)(srcFrame.data),srcFrame.cols,srcFrame.rows, QImage::Format_RGB888); //简单地转换一下为Image对象，rgbSwapped是为了显示效果色彩好一些。
            ui->label_video->setPixmap(QPixmap::fromImage(qimg));
            ui->label_video->show();
        }


    }
    videoCapture.release();
    return 0;
}


/**
 * @brief MainWindow::on_pushButton_open_clicked 点击选择文件
 * @author HezeLao
 * @date 2018-12-28 21:50
 */
void MainWindow::on_pushButton_open_clicked(){
    cout << "debug:开始选择文件事件" << endl;
    auto fileName = QFileDialog::getOpenFileName(nullptr, "Open Video!", QDir::currentPath(), "video files(*.avi;*.mp4;*.wmv;*.flv;);;all files(*.*)");
    if(fileName.isEmpty()){
        cout << "debug:打开文件失败" << endl;
    }
    else{
        videoPath = fileName.toStdString();
        isOpen = true;
        cout << "debug:打开文件成功" << endl;
        cout << "debug:路径是:" << videoPath << endl;
    }
    cout << "debug:结束选择文件事件" << endl;
}


/**
 * @brief MainWindow::on_pushButton_start_line_clicked 点击开始检测车道线
 * @author HezeLao
 * @date 2018-12-28 21:50
 */
void MainWindow::on_pushButton_start_line_clicked(){
    //正在运行检测中
    if(isStart == true && isPause == false){
        cout << "debug:正在检测中" << endl;
        return;
    }

    //从暂停恢复检测
    if(isPause == true){
        isPause = false;
        cout << "debug:从暂停恢复检测" << endl;
        return;
    }

    //初次点击开始，检查是否选择目标视频
    if(isOpen == true){
        if(!MyRunner(false)){
            //成功开始检测
            cout << "debug:成功开始检测" << endl;
            isOpen = false;
            isStart = false;
            isPause = false;
            isStop = false;
        }
        else{
            //检测失败
            cout << "debug:检测失败" << endl;
        }
    }
    else{
        cout << "debug:未选择文件" << endl;
        QMessageBox box(QMessageBox::Warning,"warning",QString::fromLocal8Bit("请先选择视频文件"));
        box.setStandardButtons (QMessageBox::Ok|QMessageBox::Cancel);
        box.setButtonText (QMessageBox::Ok,QString::fromLocal8Bit("打开文件"));
        box.setButtonText (QMessageBox::Cancel,QString::fromLocal8Bit("取消"));
        auto select = box.exec ();
        if(select == QMessageBox::Ok){
            on_pushButton_open_clicked();
        }
        return;
    }

    cout << "debug:结束检测事件" << endl;
}


/**
 * @brief MainWindow::on_pushButton_sign_clicked 点击开始检测
 * @author HezeLao
 * @date 2018-12-28 21:50
 */
void MainWindow::on_pushButton_start_sign_clicked(){
    //正在运行检测中
    if(isStart == true && isPause == false){
        cout << "debug:正在检测中" << endl;
        return;
    }

    //从暂停恢复检测
    if(isPause == true){
        isPause = false;
        cout << "debug:从暂停恢复检测" << endl;
        return;
    }

    //初次点击开始，检查是否选择目标视频
    if(isOpen == true){
        if(!MyRunner(true)){
            //成功开始检测
            cout << "debug:成功开始检测" << endl;
            isOpen = false;
            isStart = false;
            isPause = false;
            isStop = false;
        }
        else{
            //检测失败
            cout << "debug:检测失败" << endl;
        }
    }
    else{
        cout << "debug:未选择文件" << endl;
        QMessageBox box(QMessageBox::Warning,"warning",QString::fromLocal8Bit("请先选择视频文件"));
        box.setStandardButtons (QMessageBox::Ok|QMessageBox::Cancel);
        box.setButtonText (QMessageBox::Ok,QString::fromLocal8Bit("打开文件"));
        box.setButtonText (QMessageBox::Cancel,QString::fromLocal8Bit("取消"));
        auto select = box.exec ();
        if(select == QMessageBox::Ok){
            on_pushButton_open_clicked();
        }
        return;
    }

    cout << "debug:结束检测事件" << endl;
}


/**
 * @brief MainWindow::on_pushButton_pause_clicked 点击暂停
 * @author HezeLao
 * @date 2018-12-28 21:50
 */
void MainWindow::on_pushButton_pause_clicked(){
    //在已经开始检测的条件下可以暂停
    if(isStart == true){
        isPause = true;
        cout << "debug:已暂停" << endl;
        return;
    }
    else{
        //未在运行检测，无法暂停
        QMessageBox::warning(NULL, "warning", QString::fromLocal8Bit("未在运行检测"), QMessageBox::Yes);
        cout << "debug:未在运行检测，无法暂停" << endl;
    }
}


/**
 * @brief MainWindow::on_pushButton_stop_clicked 点击结束
 * @author HezeLao
 * @date 2018-12-28 21:50
 */
void MainWindow::on_pushButton_stop_clicked(){
    //未在运行检测，无法结束
    if(isStart == false){
        QMessageBox::warning(NULL, "warning", QString::fromLocal8Bit("未在运行检测"), QMessageBox::Yes);
        cout << "debug:未在运行检测，无法结束" << endl;
    }
    else{
        //执行结束事件
        isOpen = false;
        isStart = false;
        isPause = false;
        isStop = true;
        cout << "debug:执行结束事件" << endl;
    }
}


/**
 * @brief MainWindow::closeEvent 关闭窗口事件
 * @param event 事件
 * @author HezeLao
 * @date 2018-12-28 22:20
 */
void MainWindow::closeEvent(QCloseEvent *event){
    auto isExit = QMessageBox::information(this, "Tips", QString::fromLocal8Bit("确认退出？"), QMessageBox::Yes | QMessageBox::No);
    if(isExit == QMessageBox::Yes)
        event->accept();
    else
        event->ignore();
}
