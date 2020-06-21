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

	void on_pbServerGitLab_clicked();

	void on_lePassword_textChanged(const QString &arg1);

	void on_leUsername_textChanged(const QString &arg1);

	void on_pbLoginNext_clicked();

	void on_serverStateChanged(int server_index, int state);

	void on_teamsView_activated(const QModelIndex &index);

private:
	Ui::AppWidget *ui;
};

#endif // APPWIDGET_H
