#ifndef QRDIALOG_H
#define QRDIALOG_H

#include <QDialog>
#include <QGraphicsScene>
namespace Ui {
class QrDialog;
}


/**
 * @brief The QrDialog class represents dialog shows QR code.
 */
class QrDialog : public QDialog
{
    Q_OBJECT
    QGraphicsScene m_scene;		//! scene with qr picture
    QPixmap m_code;				//! qr picture

public:
    explicit QrDialog( const std::string &qr_code, QWidget *parent = 0 );	//!
    ~QrDialog();

    void SetQrCode( const std::string &qr_code );

private:
    Ui::QrDialog *ui;
};

#endif // QRDIALOG_H
