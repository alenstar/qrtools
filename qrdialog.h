#ifndef QRDIALOG_H
#define QRDIALOG_H

#include <QDialog>
#include <QCameraInfo>
#include <QCamera>
#include <QCameraImageCapture>
#include <QCameraViewfinder>
#include <QMediaRecorder>
#include <QZXing.h>
#include <qrcodegen/cpp/QrCode.hpp>

using namespace qrcodegen;
namespace Ui {
class QrDialog;
}

class QrDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QrDialog(QWidget *parent = 0);
    ~QrDialog();

    void setCamera(const QCameraInfo *cameraInfo);

    void startCamera();
    void stopCamera();
    void updateCaptureMode();
    void setExposureCompensation(int index);
    void displayRecorderError();
public slots:
    void updateCameraState(QCamera::State state);
    void updateRecorderState(QMediaRecorder::State state);

    void displayCaptureError(int id, const QCameraImageCapture::Error error, const QString &errorString);
    void updateLockStatus(QCamera::LockStatus status, QCamera::LockChangeReason reason);
    void displayCameraError();
    void readyForCapture(bool ready);
    void displayCapturedImage();
    void takeImage();
    void imageSaved(int id, const QString &fileName);
    void displayViewfinder();
    void updateCameraDevice(QAction *action);
    void processCapturedImage(int requestId, const QImage &img);
protected:

private slots:
    void on_btnFile_clicked();

    void on_btnDecode_clicked();

    void on_btnEncode_clicked();

    void on_btnSave_clicked();

    void on_btnCapture_clicked();

    void on_method_currentIndexChanged(int index);

private:
    Ui::QrDialog *ui;
    QCamera* camera_;
    QImage* image_;
    QZXing zxdecoder;

      QCameraImageCapture *imageCapture_ = nullptr;
      QMediaRecorder* mediaRecorder_ = nullptr;

      QImageEncoderSettings imageSettings_;
      QString videoContainerFormat_;

      bool isCapturingImage_ = false;

      QCameraViewfinder* vfinder_ = nullptr;
};

#endif // QRDIALOG_H
