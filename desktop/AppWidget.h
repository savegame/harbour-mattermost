#ifndef APPWIDGET_H
#define APPWIDGET_H

#include <QWidget>

namespace Ui {
class AppWidget;
}

class ChannelsFilterProxy;

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

	void on_teamsView_clicked(const QModelIndex &index);

	void on_channelsView_clicked(const QModelIndex &index);

	void on_sendMesage(const QString &text);

	void on_channelFilter_textChanged(const QString &text);

private:
	Ui::AppWidget *ui;

	int current_server_index = 0;
	int current_team_index = 0;
	int current_channel_type = 0;
	int current_channel_index = 0;

	/** filtering list of channels  */
	ChannelsFilterProxy *m_channelsFilterProxy = nullptr;
};

#endif // APPWIDGET_H
