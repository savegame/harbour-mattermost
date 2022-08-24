#include "AppWindow.h"
#include "ui_AppWindow.h"

AppWindow::AppWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AppWindow)
{
	ui->setupUi(this);
	ui->quickView->setSource(QUrl("qrc:/qml/main.qml"));
	ui->quickView->setResizeMode(QQuickWidget::SizeRootObjectToView);
}

AppWindow::~AppWindow()
{
	delete ui;
}
