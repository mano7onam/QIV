#include "imgviewer.h"
#include <QDebug>
#include <QMessageBox>
#include <QPrintDialog>
#include <QFileDialog>
#include <QWheelEvent>
#include <QtMath>
#include <QMatrix>

#include <random>
#include "algorithms.h"
#include "kernels.h"

ImgViewer::ImgViewer(QWidget *parent) :
    QGraphicsView(parent), m_rotateAngle(0), m_IsFitWindow(false), m_IsViewInitialized(false)
{
    m_scene = new QGraphicsScene(this);
    this->setScene(m_scene);
    this->setBackgroundBrush(QBrush(QColor(38,38,38,255),Qt::SolidPattern));
    this->setDragMode(NoDrag);
}

bool ImgViewer::loadFile(const QString &strFilePath, QString &strError)
{
    // clear previous and reset
    resetView();

    m_image.load(strFilePath);
    if(m_image.isNull()){
        strError = QObject::tr("Cannot load %1.").arg(strFilePath);
        return false;
    }

    m_fileName = strFilePath;
    m_pixmap = QPixmap::fromImage(m_image);
    m_pixmapItem = m_scene->addPixmap(m_pixmap); // add pixmap to scene and return pointer to pixmapItem
    m_scene->setSceneRect(m_pixmap.rect());      // set scene rect to image
    this->centerOn(m_pixmapItem);                // ensure item is centered in the view.

    // preserve fitWindow if activated
    if(m_IsFitWindow)
        fitWindow();
    else
        this->setDragMode(ScrollHandDrag);

    m_IsViewInitialized = true;
    return true;
}

void ImgViewer::resetView()
{
    if(m_image.isNull()){
        return;
    }

    m_scene->clear();
    m_image = QImage();
    m_fileName.clear();
    m_rotateAngle = 0;
    this->setDragMode(NoDrag);
    this->resetTransform();
}

void ImgViewer::reactToFitWindowToggle(bool checked)
{
    m_IsFitWindow = checked;
    if(checked)
        fitWindow();
    else
        originalSize();
}

void ImgViewer::fitWindow()
{
    if(m_image.isNull())
        return;

    this->setDragMode(NoDrag);
    this->resetTransform();
    QPixmap px = m_pixmap; // scale a copy to viewsize (scaling the original results in blurred image)
    px = px.scaled(QSize(this->width(),this->height()),Qt::KeepAspectRatio,Qt::SmoothTransformation);
    m_pixmapItem->setPixmap(px);
    m_scene->setSceneRect(px.rect());
}

void ImgViewer::originalSize()
{
    if(m_image.isNull())
        return;

    this->setDragMode(ScrollHandDrag);
    m_pixmap = m_pixmap.scaled(QSize(m_image.width(),m_image.height()),Qt::KeepAspectRatio,Qt::SmoothTransformation);
    m_pixmapItem->setPixmap(m_pixmap);
    m_scene->setSceneRect(m_pixmap.rect());
    this->centerOn(m_pixmapItem);
}

void ImgViewer::rotateView(const int nVal)
{
    if(m_image.isNull()){
        return;
    }

    // rotate view
    this->rotate(nVal);
    //this->show(); // this necessary?

    // update angle
    m_rotateAngle += nVal;  // a=a+(-90)== -90

    // reset angle
    if(m_rotateAngle >= 360 || m_rotateAngle <= -360){
        m_rotateAngle =0;
    }
}

void ImgViewer::printView()
{
    if(m_image.isNull()){
        return;
    }
#ifndef QT_NO_PRINTER
    if (QPrintDialog(&printer).exec() == QDialog::Accepted) {
        QPainter painter(&printer);
        painter.setRenderHint(QPainter::Antialiasing);
        m_scene->render(&painter);
    }
#endif
}

bool ImgViewer::saveViewToDisk(QString &strError)
{
    if (m_image.isNull()) {
        strError = QObject::tr("Save failed.");
        return false;
    }

    // save a copy
    QImage imageCopy = m_image;

    // Output file dialog
    QString fileFormat = getImageFormat(m_fileName);
    QString strFilePath = QFileDialog::getSaveFileName(
               this,
               tr("Save File"),
               QDir::homePath(),
               fileFormat);


    // If Cancel is pressed, getSaveFileName() returns a null string.
    if (strFilePath=="") {
        strError = QObject::tr("");
        return false;
    }

    // ensure output path has proper extension
    if (!strFilePath.endsWith(fileFormat)) {
         strFilePath += "."+fileFormat;
    }

    // save image in modified state
    if (isModified()) {
        QTransform t;
        t.rotate(m_rotateAngle);
        imageCopy = imageCopy.transformed(t, Qt::SmoothTransformation);
    }

    // quality factor (-1 default, 100 max)
    // note: -1 is about 4 times smaller than original, 100 is larger than original
    if (!imageCopy.save(strFilePath,fileFormat.toLocal8Bit().constData(), 100)) {
        strError = QObject::tr("Save failed.");
        return false;
    }
    return true;
}

QString ImgViewer::getImageFormat(QString strFileName)
{
    QString str = strFileName.mid(strFileName.lastIndexOf(".")+1,-1);
    if(str.toLower() == "tif")
        str = "tiff";
    return str;
}

// preserve fitWindow state on window resize
void ImgViewer::resizeEvent(QResizeEvent *event)
{
    if(!m_IsViewInitialized)
        return;

    if(m_IsFitWindow)
        fitWindow();
    else {
        QGraphicsView::resizeEvent(event);  // call base implementation
        this->centerOn(m_pixmapItem);
    }
}

// scale image on wheelEvent
void ImgViewer::wheelEvent(QWheelEvent *event)
{
    // prevent zooming when fitWindow is active
    if(m_IsFitWindow)
        return;

    this->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    // zoomIn factor
    qreal factor = 1.15;

    // zoomOut factor
    if(event->delta() < 0)
        factor = 1.0 / factor;

    // scale  the View
    this->scale(factor,factor);
    event->accept();
}

void ImgViewer::drawChangedImage()
{
    m_pixmap = QPixmap::fromImage(m_image);
    m_pixmapItem = m_scene->addPixmap(m_pixmap); // add pixmap to scene and return pointer to pixmapItem
    m_scene->setSceneRect(m_pixmap.rect());      // set scene rect to image
}

void ImgViewer::applyRandomBlurAlgorithm()
{
    std::mt19937 mt_rand(time(0));
    auto dice_rand = std::bind(std::uniform_int_distribution<int>(0, 4), mt_rand);
    for (int i = 0; i < m_image.width(); ++i)
    {
        for (int j = 0; j < m_image.height(); ++j)
        {
            if (dice_rand() == 0)
            {
                int allR = 0;
                int allG = 0;
                int allB = 0;
                int cnt = 0;
                for (int ci = i - 3; ci <= i + 3; ++ci) {
                    for (int cj = j - 3; cj <= j + 3; ++cj) {
                        if (ci < 0 || cj < 0 || ci >= m_image.width() || cj >= m_image.height()) continue;
                        allR += qRed(m_image.pixel(ci, cj));
                        allG += qGreen(m_image.pixel(ci, cj));
                        allB += qBlue(m_image.pixel(ci, cj));
                        cnt++;
                    }
                }
                allR /= cnt;
                allG /= cnt;
                allB /= cnt;
                m_image.setPixel(i, j, qRgb(allR, allG, allB));
            }
        }
    }

    drawChangedImage();
}

void ImgViewer::applyCannyAlgorithm()
{
    auto grayscale = m_image.convertToFormat(QImage::Format_Grayscale8);
    m_image = algorithms::canny(grayscale, 1, 40, 120);
    drawChangedImage();
}

void ImgViewer::applyGaborFilter()
{
    QImage grayscale = m_image.convertToFormat(QImage::Format_Grayscale8);
    double lambda = 3;
    double gamma = 0.1;
    double sigma = 0.56 * lambda;
    double theta = M_PI / 3;
    double phi = 0;
    auto kernel = algorithms::getGaborKernel(sigma, theta, lambda, gamma, phi);
    m_image = algorithms::convolution(kernel, grayscale);
    drawChangedImage();
}
