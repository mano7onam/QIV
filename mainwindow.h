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
};

#endif // MAINWINDOW_H
