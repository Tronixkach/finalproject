#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QMutex>
#include <QVector>
#include <QElapsedTimer>
#include <atomic>

using namespace std;


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class ThreadWorker : public QThread
{
    Q_OBJECT
public:
    unsigned long long startnum;
    unsigned long long endnum;
    std::atomic<bool> *isstopped;

    unsigned long long Longestnum = 0;
    unsigned long length = 0;
    bool overflowOccurred = false;

    ThreadWorker(unsigned long long start, unsigned long long end, std::atomic<bool> *stopped)
        : startnum(start), endnum(end), isstopped(stopped) {}

protected:
    void run() override;
};


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void startCalc();
    void stopCalc();
    void TableOfThreads();
    void on_slider_valueChanged(int value);

private:
    Ui::MainWindow *ui;

    vector<ThreadWorker*> workers;
    std::atomic<bool> isstopped{false};
    QElapsedTimer timer;

    void ResetUI();

};
#endif // MAINWINDOW_H
