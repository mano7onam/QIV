#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>


namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QLabel *m_infoLabel;
    QString formatByteSize(int nBytes);
    void enableControls(bool bEnable);
    void updateStatusBarInfo(QString strFile);

    struct ImageName {
        QImage image;
        QString name;
        ImageName() {}
        ImageName(QImage image, QString name) : image(image), name(name) {}
        ImageName(QImage &&image, QString &&name) : image(image), name(name) {}
    };

    std::vector<ImageName> images;
    int curImage = 0;

    bool loadFile(QString strFilePath, QString &strErr);
    void openImage(QString strFilePath);
    void updateCurrentImage();

private slots:
    void openImage();
    void openImages();
    void closeImage();
    void printImage();
    void saveImage();
    void rotateImage();
    void on_actionAbout_triggered();
    void on_actionApplyKanny_triggered();
    void on_actionGarborFilter_triggered();
    void on_actionopenSeveralImages_triggered();
    void on_actionNextImage_triggered();
};

#endif // MAINWINDOW_H
