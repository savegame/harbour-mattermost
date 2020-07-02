#ifndef APPWIDGET_H
#define APPWIDGET_H

#include <QWidget>
#include <QNetworkCookie>
#include <MattermostQt.h>

namespace Ui {
class AppWidget;
}

class ChannelsFilterProxy;

class AppWidget : public QWidget
{
	Q_OBJECT
public:
	struct Attachment {
		float   progress = 0;
		QString path;
		QString	id;
		MattermostQt::FileType type;

		Attachment(QString path, MattermostQt::FileType type)
		{
			this->path = path;
			this->type = type;
		}
	};
	typedef QSharedPointer<Attachment> AttachmentPtr;
public:
	explicit AppWidget(QWidget *parent = nullptr);
	~AppWidget();


	void setToken(QString tkn);


protected:
	virtual void mouseMoveEvent(QMouseEvent *e) override;

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

	void on_pbServerGitLab_clicked(bool checked);
	
	void on_authenticationStack_currentChanged(int index);
	
	void on_tokenChanged(QString token);
	
	void connectionError(int code, QString message, int server_index);
	
	void on_cookieAdded(const QNetworkCookie &cookie);

	void on_teamAdded(MattermostQt::TeamPtr team);

	void on_fileUploadProgress(QString data, int progress);

	void on_fileUploaded(int server_index, int file_sc_index);

	void on_attachButton_clicked();

Q_SIGNALS:
	void tokenChanged(QString token);
	
private:
	Ui::AppWidget *ui;

	int current_server_index = 0;
	int current_team_index = 0;
	int current_channel_type = 0;
	int current_channel_index = 0;
	
	QString token;

	/** filtering list of channels  */
	ChannelsFilterProxy *m_channelsFilterProxy = nullptr;

	QList<AttachmentPtr> m_attaches;
	/** if we somthing not send, it shoulb stored in draft */
	QHash<QString, QString> m_drafts;
};

#endif // APPWIDGET_H
