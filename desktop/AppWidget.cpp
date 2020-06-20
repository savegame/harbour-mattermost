#include "AppWidget.h"
#include "ui_AppWidget.h"

AppWidget::AppWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AppWidget)
{
	ui->setupUi(this);
}

AppWidget::~AppWidget()
{
	delete ui;
}

void AppWidget::on_leServerUrl_textChanged(const QString &text)
{
//    if( ui->leServerUrl->text() )
	if( !text.isEmpty() )
		ui->pbServerNext->setEnabled(true);
	else
		ui->pbServerNext->setEnabled(false);
}

void AppWidget::on_pbServerNext_clicked()
{
	ui->authenticationStack->setCurrentIndex(1);
}
