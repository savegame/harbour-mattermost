#ifndef APPWIDGET_H
#define APPWIDGET_H

#include <QWidget>

namespace Ui {
class AppWidget;
}

class AppWidget : public QWidget
{
	Q_OBJECT

public:
	explicit AppWidget(QWidget *parent = nullptr);
	~AppWidget();


private slots:
	void on_leServerUrl_textChanged(const QString &arg1);

	void on_pbServerNext_clicked();

private:
	Ui::AppWidget *ui;
};

#endif // APPWIDGET_H
