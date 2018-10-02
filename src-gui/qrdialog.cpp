#include "qrdialog.h"
#include "ui_qrdialog.h"
#include <qrencode.h>
#include <memory>

QrDialog::QrDialog( const std::string &qr_code, QWidget *parent )
:	QDialog( parent ), ui( new Ui::QrDialog )
{
	ui->setupUi( this );
	
	SetQrCode( qr_code );
}

QrDialog::~QrDialog()
{
	delete ui;
}

void QrDialog::SetQrCode( const std::string &qr_code ) {
	std::unique_ptr<QRcode, void(*)(QRcode*)> qr( QRcode_encodeString( qr_code.c_str(), 1, QR_ECLEVEL_M, QR_MODE_8, 1 ), QRcode_free );
	if( !qr )
		throw std::runtime_error( "Invalid QR generation!!!" );

	unsigned char rect_size = 8;
	QImage tmp( qr->width*rect_size, qr->width*rect_size, QImage::Format::Format_RGB32 );
	QColor color( Qt::white );
	tmp.fill( color );

	for( int yy = 0; yy < qr->width; yy++ )
		for( int xx= 0; xx < qr->width; xx++ )
			if( qr->data[yy*qr->width+xx] & 0x01 )
				for( int i = 0; i < rect_size; i++ )
					for( int j = 0; j < rect_size; j++ )
						tmp.setPixelColor( xx*rect_size+i, yy*rect_size+j, Qt::black );
	m_code = QPixmap::fromImage( tmp );

	m_scene.addPixmap( m_code );
	ui->QrView->setScene( &m_scene );
	ui->lblQrText->setText( QString::fromStdString( qr_code ));
}
