#include "qrdialog.h"
#include "logdef.h"
#include "ui_qrdialog.h"
#include "utils.h"

#include "easylogging++.h"

#include <QSerialPort>
#include <QSerialPortInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>
#include <vector>

#define DTYPE_NONE     0x00
#define DTYPE_CAMERA     0x01
#define DTYPE_SERIALPORT 0x02

void paintQR(QPainter &painter, const QSize sz, const QrCode &qr, QColor fg)
{
    const int s = qr.getSize() > 0 ? qr.getSize() : 1;
    const double w = sz.width();
    const double h = sz.height();
    const double aspect = w / h;
    const double size = ((aspect > 1.0) ? h : w);
    const double scale = size / (s + 2);
    painter.setPen(Qt::NoPen);
    painter.setBrush(fg);
    for (int y = 0; y < s; y++) {
        for (int x = 0; x < s; x++) {
            const int color = qr.getModule(x, y); // 0 for white, 1 for black
            if (0x0 != color) {
                const double rx1 = (x + 1) * scale, ry1 = (y + 1) * scale;
                QRectF r(rx1, ry1, scale, scale);
                painter.drawRects(&r, 1);
            }
        }
    }
}

QString asciiQR(const QrCode &qr, QString on, QString off)
{
    const int s = qr.getSize() > 0 ? qr.getSize() : 1;
    QString out = "";
    for (int y = 0; y < s; y++) {
        out = out + "\n";
        for (int x = 0; x < s; x++) {
            const int color = qr.getModule(x, y); // 0 for white, 1 for black
            out = out + (0x0 == color ? off : on);
        }
    }
    return out;
}

QrDialog::QrDialog(QWidget *parent)
    : QDialog(parent), camera_(nullptr), ui(new Ui::QrDialog)
{
    ui->setupUi(this);
    image_ = new QImage();

        ui->method->addItem("NULL", 0x00);
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    foreach (const QCameraInfo &cameraInfo, cameras) {
        LOGD("deviceName: %s", cameraInfo.deviceName().toLocal8Bit().data());
        LOGD("description: %s", cameraInfo.description().toLocal8Bit().data());
        ui->method->addItem(cameraInfo.deviceName(), DTYPE_CAMERA);
    }

    QList<QSerialPortInfo> serialports = QSerialPortInfo::availablePorts();
    foreach (const QSerialPortInfo &sp, serialports) {
        LOGD("deviceName: %s", sp.portName().toLocal8Bit().data());
        LOGD("description: %s", sp.description().toLocal8Bit().data());
        ui->method->addItem(sp.portName(), DTYPE_SERIALPORT);
    }


        ui->method->addItem("BASE64", 0x11);
        ui->method->addItem("HEX", 0x12);
        ui->method->addItem("MD5", 0x13);
        ui->method->addItem("HMAC-MD5", 0x13);
        ui->method->addItem("SHA1", 0x13);
        ui->method->addItem("SHA256", 0x14);
        ui->method->addItem("SM3", 0x15);

        ui->method->addItem("AES", 0x21);
        ui->method->addItem("DES", 0x22);
        ui->method->addItem("3DES", 0x23);
        ui->method->addItem("RSA1024", 0x24);
        ui->method->addItem("RSA2048", 0x25);
        ui->method->addItem("ECDSA192K", 0x26);
        ui->method->addItem("ECDSA256K", 0x27);

        // setCamera(QCameraInfo::defaultCamera());
        this->vfinder_ = new QCameraViewfinder(this);
        this->vfinder_->hide();
        this->ui->verticalLayout->insertWidget(this->ui->verticalLayout->count() - 2, this->vfinder_);
}

QrDialog::~QrDialog()
{
    if (image_) {
        delete image_;
    }
    if (camera_) {
        delete camera_;
    }
    delete ui;
}

void QrDialog::on_btnFile_clicked()
{
    QString filename = QFileDialog::getOpenFileName(
        this, tr("Select Image"), QString::fromLocal8Bit(qgetenv("HOME")),
        tr("Images (*.png *.jpg *.bmp)"));
    LOGD("file name: %s", filename.toLocal8Bit().data());
    if (!filename.isEmpty()) {
        QPixmap pixmap(filename);
        if (!image_->load(filename)) {
            LOGE("image load failed");
            return;
        } else {
            QImage img = image_->scaled(
                ui->qrcode->width(), ui->qrcode->height(), Qt::KeepAspectRatio);
            ui->qrcode->setPixmap(QPixmap::fromImage(img));

            // to decode
            on_btnDecode_clicked();
            /*
            zxdecoder.setDecoder(QZXing::DecoderFormat_QR_CODE |
                                 QZXing::DecoderFormat_EAN_13);
            QByteArray result = zxdecoder.decodeImage(img);
            if (!result.isNull()) {
                ui->qrtext->setPlainText(QString(result));
                LOGD("file name: %s", result.data());
            }
            */
        }
    }
}

void QrDialog::on_btnDecode_clicked()
{
    if (ui->qrcode->pixmap() == nullptr) {
        LOGD("not found pixmap");
        return;
    }
    QImage img = ui->qrcode->pixmap()->toImage();
    QByteArray result = zxdecoder.decodeImage(img);
    if (!result.isEmpty()) {
        LOGD("QR length:%d", result.size());
        bool isPrint = true;
        if(result.data()[0] < 0x20 || (unsigned char)(result.data()[0]) > 0x7f) {
                isPrint = false;
        }
        if (isPrint) {
            ui->params->setPlainText(result);
            ui->outRaw->setEnabled(true);
        } else {
            ui->outHex->setEnabled(true);
            ui->params->setPlainText(QString::fromUtf8(result.toHex()));
        }
    } else {
        LOGD("qr not found");
    }
}

void QrDialog::on_btnEncode_clicked()
{
    QString text = ui->params->toPlainText();
    if (!text.isEmpty()) {
        LOGD("TEXT: %s", text.toUtf8().data());
        // QrCode qr = QrCode::encodeText(text.toLocal8Bit().data(),
        // QrCode::Ecc::MEDIUM);
        // qr.toSvgString(4);
        std::vector<QrSegment> segs =
            QrSegment::makeSegments(text.toUtf8().data());
        QrCode qr =
            QrCode::encodeSegments(segs, QrCode::Ecc::HIGH, 5, 5, 2, false);
        // QImage qrimage(QSize(qr.getSize(), qr.getSize()),
        // QImage::Format_ARGB32_Premultiplied);
        QImage qrimage(ui->qrcode->size(), QImage::Format_ARGB32_Premultiplied);

        QPainter qrPainter(&qrimage);
        qrPainter.initFrom(this);
        qrPainter.setRenderHint(QPainter::Antialiasing, true);
        qrPainter.eraseRect(qrimage.rect());
        paintQR(qrPainter, ui->qrcode->size(), qr, QColor(0x00, 0x00, 0x00));
        qrPainter.end();

        // QImage img = qrimage.scaled(ui->qrcode->width(),
        // ui->qrcode->height(), Qt::KeepAspectRatio);
        ui->qrcode->setPixmap(QPixmap::fromImage(qrimage));

        // QString result = asciiQR(qr, "X", "O");
        // LOGD("RESULT: \n%s", result.toUtf8().data());
    }
}

void QrDialog::on_btnSave_clicked()
{
    QString filename = QFileDialog::getSaveFileName(
        this, tr("Save Image"), QString::fromLocal8Bit(qgetenv("HOME")),
        tr("Images (*.png *.jpg *.bmp)"));
    if (filename.isEmpty()) {
        return;
    }
    /*
    if (QFile::exists(filename)) {
        LOG(ERROR) << "the file is exists: " << filename;
        QMessageBox::StandardButton btn = QMessageBox::question(this,
                              tr("Overwrite the file?"), tr("The file already
    exists. Do you want to overwrite it?"),
                              QMessageBox::Ok | QMessageBox::No);
        if (btn == QMessageBox::No) {
            return;
        }
    }
    */

    QString text = ui->params->toPlainText();
    if (!text.isEmpty()) {
        LOGD("TEXT: %s", text.toUtf8().data());
        // QrCode qr = QrCode::encodeText(text.toLocal8Bit().data(),
        // QrCode::Ecc::MEDIUM);
        // qr.toSvgString(4);
        std::vector<QrSegment> segs =
            QrSegment::makeSegments(text.toUtf8().data());
        QrCode qr =
            QrCode::encodeSegments(segs, QrCode::Ecc::HIGH, 5, 5, 2, false);
        QImage qrimage(QSize(qr.getSize() * 8, qr.getSize() * 8),
                       QImage::Format_ARGB32_Premultiplied);

        QPainter qrPainter(&qrimage);
        qrPainter.initFrom(this);
        qrPainter.setRenderHint(QPainter::Antialiasing, true);
        qrPainter.eraseRect(qrimage.rect());
        paintQR(qrPainter, qrimage.size(), qr, QColor(0x00, 0x00, 0x00));
        qrPainter.end();

        if (!qrimage.save(filename, "png")) {
            LOG(ERROR) << "save image to file failed: " << filename;
            return;
        }
        // QString result = asciiQR(qr, "X", "O");
        // LOGD("RESULT: \n%s", result.toUtf8().data());
    }
}

void QrDialog::setCamera(const QCameraInfo *cameraInfo)
{
    if(cameraInfo == nullptr) {
        this->vfinder_->hide();
        this->ui->qrcode->show();
       return;
    }

    if(imageCapture_) {
        delete imageCapture_;
    }
    if (camera_) {
        camera_->stop();
    } else {
        camera_ = new QCamera(*cameraInfo);
    }


    connect(camera_, &QCamera::stateChanged, this,
            &QrDialog::updateCameraState);
    connect(camera_, QOverload<QCamera::Error>::of(&QCamera::error), this,
            &QrDialog::displayCameraError);
    //mediaRecorder = new QMediaRecorder(camera);
    //connect(mediaRecorder, &QMediaRecorder::stateChanged, this,
    //        &Camera::updateRecorderState);

    imageCapture_ = new QCameraImageCapture(camera_);

    connect(imageCapture_, &QCameraImageCapture::imageCaptured, this,
            &QrDialog::processCapturedImage);

    connect(imageCapture_, &QCameraImageCapture::readyForCaptureChanged, this,
            &QrDialog::readyForCapture);
    connect(imageCapture_, &QCameraImageCapture::imageCaptured, this,
            &QrDialog::processCapturedImage);
    connect(imageCapture_,
            QOverload<int, QCameraImageCapture::Error, const QString &>::of(
                &QCameraImageCapture::error),
            this, &QrDialog::displayCaptureError);
    connect(camera_,
            QOverload<QCamera::LockStatus, QCamera::LockChangeReason>::of(
                &QCamera::lockStatusChanged),
            this, &QrDialog::updateLockStatus);

    //ui->captureWidget->setTabEnabled(
    //    0, (camera->isCaptureModeSupported(QCamera::CaptureStillImage)));
    //ui->captureWidget->setTabEnabled(
    //    1, (camera->isCaptureModeSupported(QCamera::CaptureVideo)));

     this->camera_->setViewfinder(this->vfinder_);
     QCameraViewfinderSettings viewfinderSettings;
  // viewfinderSettings.setResolution(640, 480);
  viewfinderSettings.setMinimumFrameRate(15.0);
  viewfinderSettings.setMaximumFrameRate(30.0);
    this->camera_->setViewfinderSettings(viewfinderSettings);

    updateCaptureMode();
    camera_->start();
    this->vfinder_->show();
        this->ui->qrcode->hide();
}

void QrDialog::updateLockStatus(QCamera::LockStatus status,
                                QCamera::LockChangeReason reason)
{
    QColor indicationColor = Qt::black;

    switch (status) {
    case QCamera::Searching:
        indicationColor = Qt::yellow;
        // ui->statusbar->showMessage(tr("Focusing..."));
        // ui->lockButton->setText(tr("Focusing..."));
        break;
    case QCamera::Locked:
        indicationColor = Qt::darkGreen;
        // ui->lockButton->setText(tr("Unlock"));
        // ui->statusbar->showMessage(tr("Focused"), 2000);
        break;
    case QCamera::Unlocked:
        indicationColor = reason == QCamera::LockFailed ? Qt::red : Qt::black;
        // ui->lockButton->setText(tr("Focus"));
        if (reason == QCamera::LockFailed) {
            // ui->statusbar->showMessage(tr("Focus Failed"), 2000);
        }
    }

    // QPalette palette = ui->lockButton->palette();
    // palette.setColor(QPalette::ButtonText, indicationColor);
    // ui->lockButton->setPalette(palette);
}

void QrDialog::displayCaptureError(int id,
                                   const QCameraImageCapture::Error error,
                                   const QString &errorString)
{
    Q_UNUSED(id);
    Q_UNUSED(error);
    QMessageBox::warning(this, tr("Image Capture Error"), errorString);
    isCapturingImage_ = false;
}

  void QrDialog::startCamera()
  {
      camera_->start();
  }

  void QrDialog::stopCamera()
  {
      camera_->stop();
  }


  void QrDialog::updateCaptureMode()
  {
      //int tabIndex = ui->captureWidget->currentIndex();
      //QCamera::CaptureModes captureMode = tabIndex == 0 ? QCamera::CaptureStillImage : QCamera::CaptureVideo;
      QCamera::CaptureModes captureMode = QCamera::CaptureVideo;

      if (camera_->isCaptureModeSupported(captureMode))
          camera_->setCaptureMode(captureMode);
  }

  void QrDialog::updateCameraState(QCamera::State state)
  {
      switch (state) {
      case QCamera::ActiveState:
          //ui->actionStartCamera->setEnabled(false);
          //ui->actionStopCamera->setEnabled(true);
          //ui->captureWidget->setEnabled(true);
          //ui->actionSettings->setEnabled(true);
          break;
      case QCamera::UnloadedState:
      case QCamera::LoadedState:
          break;
          //ui->actionStartCamera->setEnabled(true);
          //ui->actionStopCamera->setEnabled(false);
          //ui->captureWidget->setEnabled(false);
          //ui->actionSettings->setEnabled(false);
      }
  }

  void QrDialog::updateRecorderState(QMediaRecorder::State state)
  {
      switch (state) {
      case QMediaRecorder::StoppedState:
          //ui->recordButton->setEnabled(true);
          //ui->pauseButton->setEnabled(true);
          //ui->stopButton->setEnabled(false);
          break;
      case QMediaRecorder::PausedState:
          //ui->recordButton->setEnabled(true);
          //ui->pauseButton->setEnabled(false);
          //ui->stopButton->setEnabled(true);
          break;
      case QMediaRecorder::RecordingState:
          //ui->recordButton->setEnabled(false);
          //ui->pauseButton->setEnabled(true);
          //ui->stopButton->setEnabled(true);
          break;
      }
  }

  void QrDialog::setExposureCompensation(int index)
  {
      camera_->exposure()->setExposureCompensation(index*0.5);
  }

  void QrDialog::displayRecorderError()
  {
      //QMessageBox::warning(this, tr("Capture Error"), mediaRecorder->errorString());
  }

  void QrDialog::displayCameraError()
  {
      QMessageBox::warning(this, tr("Camera Error"), camera_->errorString());
  }

  void QrDialog::updateCameraDevice(QAction *action)
  {
      //setCamera(qvariant_cast<QCameraInfo>(action->data()));
  }

  void QrDialog::displayViewfinder()
  {
      //cui->stackedWidget->setCurrentIndex(0);
  }

  void QrDialog::displayCapturedImage()
  {
      // ui->stackedWidget->setCurrentIndex(1);
  }

  void QrDialog::readyForCapture(bool ready)
  {
      //ui->takeImageButton->setEnabled(ready);
  }

  void QrDialog::imageSaved(int id, const QString &fileName)
  {
      Q_UNUSED(id);
      // ui->statusbar->showMessage(tr("Captured \"%1\"").arg(QDir::toNativeSeparators(fileName)));

      isCapturingImage_ = false;
      //if (applicationExiting)
      //    close();
  }

 void QrDialog::takeImage()
  {
      isCapturingImage_ = true;
      imageCapture_->capture();
  }


  void QrDialog::processCapturedImage(int requestId, const QImage& img)
  {
      Q_UNUSED(requestId);
      QImage scaledImage = img.scaled(ui->qrcode->size(),
                                      Qt::KeepAspectRatio,
                                      Qt::SmoothTransformation);
      ui->qrcode->setPixmap(QPixmap::fromImage(scaledImage));

      // Display captured image for 4 seconds.
      // displayCapturedImage();
      // QTimer::singleShot(4000, this, &QrDialog::displayViewfinder);
  }

void QrDialog::on_btnCapture_clicked()
{
      isCapturingImage_ = true;
      imageCapture_->capture();
}

void QrDialog::on_method_currentIndexChanged(int index)
{
   QVariant v = this->ui->method->currentData();
   int type = v.toInt();
   if (type == DTYPE_CAMERA) {
      QCameraInfo cam(this->ui->method->currentText().toUtf8());
      this->setCamera(&cam);
   } else if(type == DTYPE_NONE) {
       if(this->camera_) {
           this->setCamera(nullptr);
       }
   } else {
       if(this->camera_) {
           this->setCamera(nullptr);
       }

   }
}
