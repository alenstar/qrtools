#include "qrdialog.h"
#include "ui_qrdialog.h"
#include "logdef.h"

#include "easylogging++.h"

#include <QMessageBox>
#include <QPainter>
#include <QFileDialog>

#include <vector>

void paintQR(QPainter &painter, const QSize sz, const QrCode &qr, QColor fg)
{
    const int s = qr.getSize() > 0 ? qr.getSize() : 1;
    const double w=sz.width();
    const double h=sz.height();
    const double aspect=w/h;
    const double size=((aspect>1.0)?h:w);
    const double scale=size/(s+2);
    painter.setPen(Qt::NoPen);
    painter.setBrush(fg);
    for(int y=0; y<s; y++) {
        for(int x=0; x<s; x++) {
            const int color = qr.getModule(x, y);  // 0 for white, 1 for black
            if(0x0!=color) {
                const double rx1=(x+1)*scale, ry1=(y+1)*scale;
                QRectF r(rx1, ry1, scale, scale);
                painter.drawRects(&r,1);
            }
        }
    }
}

QString asciiQR(const QrCode &qr, QString on, QString off)
{
    const int s =qr.getSize() > 0 ? qr.getSize():1;
    QString out="";
    for(int y=0; y<s; y++) {
        out = out + "\n";
        for(int x=0; x<s; x++) {
            const int color = qr.getModule(x, y);  // 0 for white, 1 for black
            out = out + (0x0==color?off:on);
        }
    }
    return out;
}


QrDialog::QrDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::QrDialog)
{
    ui->setupUi(this);
    image_ = new QImage();
}

QrDialog::~QrDialog()
{
    if(image_) {
        delete image_;
    }
    delete ui;
}

void QrDialog::on_btnFile_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Select Image"),
                QString::fromLocal8Bit(qgetenv("HOME")), tr("Images (*.png *.jpg *.bmp)"));
    LOGD("file name: %s", filename.toLocal8Bit().data());
    if(!filename.isEmpty()) {
        QPixmap pixmap(filename);
        if(!image_->load(filename)) {
           LOGE("image load failed");
           return;
        }else {
            QImage img = image_->scaled(ui->qrcode->width(),
                       ui->qrcode->height(),
                       Qt::KeepAspectRatio);
            ui->qrcode->setPixmap(QPixmap::fromImage(img));

            zxdecoder.setDecoder( QZXing::DecoderFormat_QR_CODE | QZXing::DecoderFormat_EAN_13 );
            QString result = zxdecoder.decodeImage(img);
            if (!result.isEmpty()) {
                ui->qrtext->setPlainText(result);
            }
            LOGD("file name: %s", result.toLocal8Bit().data());
        }
    }
}

void QrDialog::on_btnDecode_clicked()
{
    zxdecoder.setDecoder( QZXing::DecoderFormat_QR_CODE | QZXing::DecoderFormat_EAN_13 );
    if (ui->qrcode->pixmap() == nullptr) {
        LOGD("not found pixmap");
        return;
    }
    QImage img = ui->qrcode->pixmap()->toImage();
    QString result = zxdecoder.decodeImage(img);
    if (!result.isEmpty()) {
        ui->qrtext->setPlainText(result);
    }

}

void QrDialog::on_btnEncode_clicked()
{
    QString text = ui->qrtext->toPlainText();
    if(!text.isEmpty()) {
        LOGD("TEXT: %s", text.toUtf8().data());
        // QrCode qr = QrCode::encodeText(text.toLocal8Bit().data(), QrCode::Ecc::MEDIUM);
        // qr.toSvgString(4);
        std::vector<QrSegment> segs = QrSegment::makeSegments(text.toUtf8().data());
        QrCode qr = QrCode::encodeSegments(
                segs, QrCode::Ecc::HIGH, 5, 5, 2, false);
        // QImage qrimage(QSize(qr.getSize(), qr.getSize()), QImage::Format_ARGB32_Premultiplied);
        QImage qrimage(ui->qrcode->size(), QImage::Format_ARGB32_Premultiplied);

        QPainter qrPainter(&qrimage);
        qrPainter.initFrom(this);
        qrPainter.setRenderHint(QPainter::Antialiasing, true);
        qrPainter.eraseRect(qrimage.rect());
        paintQR(qrPainter, ui->qrcode->size(), qr, QColor(0x00, 0x00, 0x00));
        qrPainter.end();

        // QImage img = qrimage.scaled(ui->qrcode->width(), ui->qrcode->height(), Qt::KeepAspectRatio);
        ui->qrcode->setPixmap(QPixmap::fromImage(qrimage));

        // QString result = asciiQR(qr, "X", "O");
        // LOGD("RESULT: \n%s", result.toUtf8().data());
    }
}

void QrDialog::on_btnSave_clicked()
{
    QString filename = QFileDialog::getSaveFileName(this, tr("Save Image"),
                            QString::fromLocal8Bit(qgetenv("HOME")), tr("Images (*.png *.jpg *.bmp)"));
    if (filename.isEmpty()) {
        return;
    }
    /*
    if (QFile::exists(filename)) {
        LOG(ERROR) << "the file is exists: " << filename;
        QMessageBox::StandardButton btn = QMessageBox::question(this,
                              tr("Overwrite the file?"), tr("The file already exists. Do you want to overwrite it?"),
                              QMessageBox::Ok | QMessageBox::No);
        if (btn == QMessageBox::No) {
            return;
        }
    }
    */

    QString text = ui->qrtext->toPlainText();
    if(!text.isEmpty()) {
        LOGD("TEXT: %s", text.toUtf8().data());
        // QrCode qr = QrCode::encodeText(text.toLocal8Bit().data(), QrCode::Ecc::MEDIUM);
        // qr.toSvgString(4);
        std::vector<QrSegment> segs = QrSegment::makeSegments(text.toUtf8().data());
        QrCode qr = QrCode::encodeSegments(
                segs, QrCode::Ecc::HIGH, 5, 5, 2, false);
        QImage qrimage(QSize(qr.getSize() * 8, qr.getSize() * 8), QImage::Format_ARGB32_Premultiplied);

        QPainter qrPainter(&qrimage);
        qrPainter.initFrom(this);
        qrPainter.setRenderHint(QPainter::Antialiasing, true);
        qrPainter.eraseRect(qrimage.rect());
        paintQR(qrPainter, qrimage.size(), qr, QColor(0x00, 0x00, 0x00));
        qrPainter.end();

        if(!qrimage.save(filename, "png")) {
            LOG(ERROR) << "save image to file failed: " << filename;
            return;
        }
        // QString result = asciiQR(qr, "X", "O");
        // LOGD("RESULT: \n%s", result.toUtf8().data());
    }
}
