#ifndef QRDIALOG_H
#define QRDIALOG_H

#include <QDialog>
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

protected:

private slots:
    void on_btnFile_clicked();

    void on_btnDecode_clicked();

    void on_btnEncode_clicked();

    void on_btnSave_clicked();

private:
    Ui::QrDialog *ui;
    QImage* image_;
    QZXing zxdecoder;
};

#endif // QRDIALOG_H
