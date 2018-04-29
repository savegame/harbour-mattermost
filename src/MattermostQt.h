#ifndef MATTERMOSTQT_H
#define MATTERMOSTQT_H

#include <QObject>
#include <QString>
#include <QUrl>
#include <QMap>
#include <QNetworkAccessManager>

class MattermostQt : public QObject
{
	Q_OBJECT
public:
	MattermostQt();

	~MattermostQt();

	bool login(QString server, QString login, QString password, int api = 4);
protected Q_SLOTS:
	void replyFinished(QNetworkReply *reply);
protected:
	QMap<int, QUrl>    m_serverURL;
	QMap<int, QString> m_serverApi;
	QMap<int, QString> m_accessToken;

	QSharedPointer<QNetworkAccessManager>  m_networkManager;
};

#endif // MATTERMOSTQT_H
