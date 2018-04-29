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
	enum ReplyType : int {
		Login,
	};

	struct ServerContainer {
		ServerContainer() : m_api(4), m_trustCertificate(false) {}

		ServerContainer(QString url, QString token, int api)
		{
			m_url = url;
			m_api = api;
			m_token = token;
			m_trustCertificate = false;
		}

		QString            m_url;   /**< server URL */
		int                m_api;   /**< server API version */
		QString            m_token; /**< server access token*/
		bool               m_trustCertificate; /**< trust self signed certificate */
		QSslConfiguration  m_cert; /**< server certificate */
	};
public:
	MattermostQt();

	~MattermostQt();

	bool login(QString server, QString login, QString password, bool trustCertificate = false, int api = 4);

protected:
	bool reply_login(QNetworkReply *reply);

protected Q_SLOTS:
	void replyFinished(QNetworkReply *reply);
	void replySSLErrors(QNetworkReply *reply, QList<QSslError> errors);

protected:
	QMap<int, ServerContainer>    m_server;

	QSharedPointer<QNetworkAccessManager>  m_networkManager;
};

#endif // MATTERMOSTQT_H
