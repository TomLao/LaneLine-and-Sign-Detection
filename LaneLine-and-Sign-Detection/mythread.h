#ifndef MYTHREAD_H
#define MYTHREAD_H
#include <QThread>
#include <QMutex>
#include <QMainWindow>
#include <opencv2/opencv.hpp>

using namespace cv;

namespace Ui {
    class MainWindow;
}

class ThreadFromQThread : public QThread
{
    Q_OBJECT

signals:
    void message(const QString& info);
    void progress(int present);

public slots:
    void stopImmediately();
    void ThreadFromQThread::setVideoPath(std::string videoPath, Ui::MainWindow* &ui);

public:
    ThreadFromQThread(QObject* par);
    ~ThreadFromQThread();
    void setSomething();
    void getSomething();
    void setRunCount(int count);
    void run();
    void doSomething();

    QMutex myPause;

private:
    int m_runCount;
    QMutex myLock;
    bool myIsCanRun;
    std::string videoPath;
    Ui::MainWindow* &ui;
};


#endif // MYTHREAD_H
