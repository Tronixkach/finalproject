#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QThread>
#include <QtTypes>
#include <QString>
#include <climits>

void ThreadWorker::run()
{
    for(unsigned long long i = startnum; i <= endnum; i++)
    {
        if (isstopped->load(std::memory_order_relaxed)) {
            break;
        }

        unsigned int len = 1;
        unsigned long long n = i;
        bool localOverflow = false;

        while(n > 1)
        {
            if(n % 2 == 0) {
                n /= 2;
            } else {
                if (n > (ULLONG_MAX - 1) / 3) {
                    localOverflow = true;
                    break;
                }
                n = 3 * n + 1;
            }
            len++;
        }
        if (localOverflow) {
            overflowOccurred = true;
            break;
        }

        if (len > length) {
            length = len;
            Longestnum = i;
        }
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    int max = QThread::idealThreadCount();
    ui->spinBox->setRange(1, 2000000000);
    ui->spinBox->setValue(1000000);
    ui->slider->setRange(1, max);
    ui->btn_stop->setEnabled(false);

    connect(ui->btn_start, &QPushButton::clicked, this, &MainWindow::startCalc);
    connect(ui->btn_stop, &QPushButton::clicked, this, &MainWindow::stopCalc);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::startCalc()
{
    isstopped.store(false, std::memory_order_relaxed);

    ui->btn_start->setEnabled(false);
    ui->btn_exit->setEnabled(false);
    ui->btn_stop->setEnabled(true);
    ui->spinBox->setEnabled(false);
    ui->slider->setEnabled(false);

    ui->textEdit->clear();
    ui->textEdit->append("Розрахунки запущено...");

    unsigned long long val = ui->spinBox->value();
    int streams = ui->slider->value();

    unsigned long long rangeForThreads = val / streams;
    timer.start();

    for(int i = 0; i < streams; i++)
    {
        unsigned long long start = rangeForThreads * i + 1;
        unsigned long long end = (i == streams - 1) ? val : (start + rangeForThreads - 1);

        ThreadWorker *worker = new ThreadWorker(start, end, &isstopped);

        workers.push_back(worker);
        connect(worker, &QThread::finished, this, &MainWindow::TableOfThreads);
        worker->start();
    }
}

void MainWindow::stopCalc()
{
    isstopped.store(true, std::memory_order_relaxed);
    ui->textEdit->append("Обчислення закінченно користувачем");
}

void MainWindow::TableOfThreads()
{
    if (workers.empty()) return;

    for(auto w : workers)
    {
        if(!w->isFinished()) return;
    }

    qint64 elapsed = timer.elapsed();
    unsigned long long BestGlobalNum = 0;
    unsigned long GlobalLenght = 0;
    bool overflowError = false;

    for(auto w : workers)
    {
        if(w->overflowOccurred) overflowError = true;
        if(w->length > GlobalLenght)
        {
            GlobalLenght = w->length;
            BestGlobalNum = w->Longestnum;
        }
        w->deleteLater();
    }

    workers.clear();

    if(overflowError) {
        ui->textEdit->append("Сталося переповнення");
    }
    else {
        ui->textEdit->append(QString("----РЕЗУЛЬТАТ----\n"
                                     "Найдовше число: %1 \n"
                                     "Довжина ланцюга: %2 \n"
                                     "Скільки часу знадобилося на це: %3 мілісек \n")
                                 .arg(BestGlobalNum).arg(GlobalLenght).arg(elapsed));
    }
    ResetUI();
}

void MainWindow::on_slider_valueChanged(int value)
{
    ui->label_2->setText(QString::number(value));
}

void MainWindow::ResetUI()
{
    ui->btn_start->setEnabled(true);
    ui->btn_exit->setEnabled(true);
    ui->btn_stop->setEnabled(false);
    ui->spinBox->setEnabled(true);
    ui->slider->setEnabled(true);
}