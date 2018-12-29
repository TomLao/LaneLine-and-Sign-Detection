#include "mythread.h"
#include <QDebug>

ThreadFromQThread::ThreadFromQThread(QObject* par) : QThread(par)
,m_runCount(20)
{

}

ThreadFromQThread::~ThreadFromQThread()
{
    qDebug() << "ThreadFromQThread::~ThreadFromQThread()";
}


/**
 * @brief ThreadFromQThread::stopImmediately 终止线程
 * @author HezeLao
 * @date 2018-12-28 15:40
 */
void ThreadFromQThread::stopImmediately(){
    QMutexLocker locker(&myLock);
    myIsCanRun = false;
}

void ThreadFromQThread::setSomething()
{
    msleep(500);
    QString str = QString("%1->%2,thread id:%3").arg(__FUNCTION__).arg(__FILE__).arg((int)QThread::currentThreadId());
    emit message(str);
}

void ThreadFromQThread::getSomething()
{
    msleep(500);
    emit message(QString("%1->%2,thread id:%3").arg(__FUNCTION__).arg(__FILE__).arg((int)QThread::currentThreadId()));
}

void ThreadFromQThread::setRunCount(int count)
{
    m_runCount = count;
    emit message(QString("%1->%2,thread id:%3").arg(__FUNCTION__).arg(__FILE__).arg((int)QThread::currentThreadId()));
}



void ThreadFromQThread::setVideoPath(std::string videoPath, Ui::MainWindow* &ui){
    this->videoPath = videoPath;
    this->ui = ui;
}
void ThreadFromQThread::doSomething()
{
    msleep(500);
    emit message(QString("%1->%2,thread id:%3").arg(__FUNCTION__).arg(__FILE__).arg((int)QThread::currentThreadId()));
}

void ThreadFromQThread::run()
{
    int count = 0;
    myIsCanRun = true;  //标记可以运行
//    QString str = QString("%1->%2,thread id:%3").arg(__FILE__).arg(__FUNCTION__).arg((int)QThread::currentThreadId());
//    emit message(str);
//    while(1)
//    {
//        sleep(1);
//        ++count;
//        emit progress(((float)count / m_runCount) * 100);
//        emit message(QString("ThreadFromQThread::run times:%1").arg(count));
//        doSomething();
//        if(m_runCount == count)
//        {
//            break;
//        }
//        QMutexLocker locker(&myLock);

        //获取帧数，不然会播放速度不对
    VideoCapture videoCapture(this->videoPath);
    Mat srcFrame;
        int length = videoCapture.get(CAP_PROP_FRAME_COUNT)/videoCapture.get(CAP_PROP_FPS);
        double FPS = videoCapture.get(CAP_PROP_FPS);
        Mat res;
        while(1){
            waitKey(1000.0 / FPS);
            this->myPause.lock();

            videoCapture>>srcFrame;

//            if(srcFrame.empty() || isStop == true){
//                ui->label_video->clear();   //视频结束或按下结束按钮，清理播放窗口，退出
//                videoCapture.release();
//                break;
//            }

            cv::resize(srcFrame, srcFrame, Size(576, 324), 0, 0, INTER_LINEAR);
            //TODO:从这里出发执行检测算法
            cvtColor(srcFrame, srcFrame, CV_BGR2RGB);

            QImage qimg = QImage((const uchar*)(res.data),srcFrame.cols,srcFrame.rows, QImage::Format_RGB888); //简单地转换一下为Image对象，rgbSwapped是为了显示效果色彩好一些。
    //        QPixmap fitpixmap = QPixmap::fromImage(qimg).scaled(576, 324, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);  // 饱满填充
    //        ui->label_video->setPixmap(fitpixmap);
            ui->label_video->setPixmap(QPixmap::fromImage(qimg));
            ui->label_video->show();
            this->myPause.unlock();
        }
        videoCapture.release();
        if(myIsCanRun == false){    //在每次循环判断是否可以运行，如果不想就退出循环
            return;
        }
//    }
}


