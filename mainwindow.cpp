#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "aboutdlg.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QDebug>
#include <QDateTime>

#include <iostream>
#include <cmath>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_infoLabel = new QLabel(this);
    m_infoLabel->setStyleSheet("QLabel {padding-left:3px;}");
    this->setStyleSheet("QStatusBar::item {border:none;}");

    // statusbar info left (color settings)
    QPalette palette;
    palette.setColor( QPalette::WindowText, "#cccccc");
    ui->statusBar->setPalette(palette);

    ui->statusBar->addWidget(m_infoLabel);

    connect(ui->actionOpen, SIGNAL(triggered()), this, SLOT(openImage()));   
    connect(ui->actionClear, SIGNAL(triggered()), this, SLOT(closeImage()));
    connect(ui->actionFitWindow, SIGNAL(toggled(bool)), ui->graphicsView, SLOT(reactToFitWindowToggle(bool)));  
    connect(ui->actionPrint, SIGNAL(triggered()), this, SLOT(printImage()));   
    connect(ui->actionSave,SIGNAL(triggered()), this, SLOT(saveImage()));    
    connect(ui->actionRotate_Left, SIGNAL(triggered()), this, SLOT(rotateImage())); 
    connect(ui->actionRotate_right, SIGNAL(triggered()), this, SLOT(rotateImage()));

    enableControls(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::enableControls(bool bEnable)
{ 
    ui->actionClear->setEnabled(bEnable);
    ui->actionPrint->setEnabled(bEnable);
    ui->actionRotate_Left->setEnabled(bEnable);
    ui->actionRotate_right->setEnabled(bEnable);
    ui->actionSave->setEnabled(bEnable);
    ui->actionFitWindow->setEnabled(bEnable);
}

void MainWindow::openImages()
{
    auto strFiles = QFileDialog::getOpenFileNames(
                this,
                tr("Open Files"),
                "../QIV/Data",
                tr("Images (*.png *.jpg *.bmp *.tiff *.tif)"));
    for (auto& strFile : strFiles) {
        std::cout << strFile.toStdString() << std::endl;
        openImage(strFile);
    }
}

bool MainWindow::loadFile(QString strFilePath, QString &errStr) {
    QImage image;
    image.load(strFilePath);
    if (image.isNull()) {
        errStr = QObject::tr("Cannot load %1.").arg(strFilePath);
        return false;
    }

    ui->graphicsView->setImage(image, strFilePath);
    images.emplace_back(image, strFilePath);

    updateStatusBarInfo(strFilePath);
    enableControls(true);

    return true;
}

void MainWindow::openImage(QString strFilePath) {
    curImage = 0;
    if (!strFilePath.isEmpty()) {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        QString strError;
        if (!loadFile(strFilePath, strError)) {
            QApplication::restoreOverrideCursor();
            QMessageBox::information(this,tr("Error"),tr(strError.toLocal8Bit().constData())); //"Error displaying image."
            return;
        }
        QApplication::restoreOverrideCursor();
    }
}

void MainWindow::openImage()
{
    QString strFilePath = QFileDialog::getOpenFileName(
                this,
                tr("Open File"),
                /*QDir::homePath()*/"../QIV/Data", // hardcoded (not so good)
                tr("Images (*.png *.jpg *.bmp *.tiff *.tif)"));

    images.clear();
    openImage(strFilePath);
}

void MainWindow::printImage()
{
    ui->graphicsView->printView();
}

void MainWindow::closeImage()
{
    ui->graphicsView->resetView();
    m_infoLabel->setText("");
    enableControls(false);
}

void MainWindow::saveImage()
{
    QString strError;
    if (!ui->graphicsView->saveViewToDisk(strError)) {
        QApplication::restoreOverrideCursor();
        if (!strError.isEmpty()) {
            QMessageBox::information(this,tr("Error"),strError);
        }
        return;
    }
}

void MainWindow::rotateImage()
{
    QObject* obj = sender();
    if (obj == ui->actionRotate_Left) {
        ui->graphicsView->rotateView(-90);
    } else {
        ui->graphicsView->rotateView(90);
    }
}

QString MainWindow::formatByteSize(int nBytes)
{
    int B = 1;
    int KB = 1024 * B;
    int MB = 1024 * KB;
    int GB = 1024 * MB;
    QString str;

    if(nBytes > GB)         str = QString::number(nBytes/GB, 'f', 2) + " GB";
    else if(nBytes > MB)    str = QString::number(nBytes/MB, 'f', 2) + " MB";
    else if(nBytes > KB)    str = QString::number(nBytes/KB, 'f', 2) + " KB";
    else                    str = QString::number(nBytes, 'f', 2) + " B";

    return str;
}

void MainWindow::updateStatusBarInfo(QString strFile)
{
    QFileInfo imgInfo(strFile);
    QDateTime dtLastModified = imgInfo.lastModified();
    QString strStatusInfo = QString(strFile+"   "+"~%1"+"   "+"%2").arg(formatByteSize(imgInfo.size())).arg(dtLastModified.toString("d/M/yyyy hh:mm:ss"));
    m_infoLabel->setText(strStatusInfo);
}

void MainWindow::on_actionAbout_triggered()
{
    AboutDlg dlg(this);
    dlg.exec();
}

void MainWindow::on_actionApplyKanny_triggered()
{
    std::cout << "Apply Canny algorithm..." << std::endl;
    ui->graphicsView->applyCannyAlgorithm();
    std::cout << "Canny algorithm applied." << std::endl;
}

void MainWindow::on_actionGarborFilter_triggered()
{
    std::cout << "Apply Gabor filter..." << std::endl;

    double ang = 0;
    double cut = M_PI / 6;

    images.clear();
    curImage = 0;

    for (int i = 0; i < 6; ++i, ang += cut) {
        auto resImage = ui->graphicsView->applyGaborFilter(ang);
        QString sErr;
        QString resFile = QStringLiteral("../results/res%1").arg(i);
        ui->graphicsView->saveImageToDisk(resImage, resFile, sErr);
        std::cout << resFile.toStdString() << ": " << sErr.toStdString() << std::endl;
        images.emplace_back(resImage, resFile);
    }

    updateCurrentImage();

    std::cout << "Gabor filter applied..." << std::endl;
}

void MainWindow::on_actionopenSeveralImages_triggered()
{
    std::cout << "Open several images:" << std::endl;
    images.clear();
    curImage = 0;
    openImages();
}

void MainWindow::updateCurrentImage() {
    if (images.empty()) return;
    ui->graphicsView->setImage(images[curImage].image, images[curImage].name);
    updateStatusBarInfo(images[curImage].name);
    std::cout << "updateCurrentImage: " << images[curImage].name.toStdString() << std::endl;
}

void MainWindow::on_actionNextImage_triggered()
{
    if (images.empty()) return;
    curImage = (curImage + 1) % images.size();
    std::cout << "increment curImage: " << curImage << std::endl;
    updateCurrentImage();
}
