#ifndef MATTERMOSTQT_H
#define MATTERMOSTQT_H

#include <QObject>
#include <QString>
#include <QUrl>
#include <QMap>
#include <QNetworkAccessManager>
#include <QtWebSockets/QWebSocket>

class MattermostQt : public QObject
{
	Q_OBJECT
public:
	enum ReplyType : int {
		Login,
		Teams
	};

	enum ConnectionError {
		WrongPassword,
		SslError
	};

	struct TeamContainer
	{
		TeamContainer()
		{
			m_create_at = 0;
			m_update_at = 0;
			m_delete_at = 0;
		}

		TeamContainer(QJsonObject &object);

		QString    m_id;
		QString    m_display_name;
		QString    m_name;
		QString    m_description;
		QString    m_type;
		QString    m_email;
		QString    m_invite_id;
		QString    m_allowed_domains;
		bool       m_allowed_open_invite;
		qlonglong  m_create_at;
		qlonglong  m_update_at;
		qlonglong  m_delete_at;
		int        m_serverId;
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

		QString                     m_url;   /**< server URL */
		int                         m_api;   /**< server API version */
		QString                     m_token; /**< server access token*/
		bool                        m_trustCertificate; /**< trust self signed certificate */
		QSslConfiguration           m_cert;  /**< server certificate */
		QSharedPointer<QWebSocket>  m_socket;/**< websocket connection */
		QList<TeamContainer>        m_teams; /**< allowed teams */
	};

public:
	MattermostQt();

	~MattermostQt();

	Q_INVOKABLE void post_login(QString server, QString login, QString password, bool trustCertificate = false, int api = 4);
	Q_INVOKABLE void get_teams(int serverId);

Q_SIGNALS:
	void serverConnected(int id);
	void connectionError(int code, QString message);
	void teamAdded(MattermostQt::TeamContainer team);

protected:
	bool reply_login(QNetworkReply *reply);
	void reply_getTeams(QNetworkReply *reply);
	void reply_error(QNetworkReply *reply);

protected Q_SLOTS:
	void replyFinished(QNetworkReply *reply);
	void replySSLErrors(QNetworkReply *reply, QList<QSslError> errors);

	void onWebSocketConnected();
	void onWebSocketSslError(QList<QSslError> errors);
	void onWebSocketError(QAbstractSocket::SocketError error);
protected:
	QMap<int, ServerContainer>    m_server;

	QSharedPointer<QNetworkAccessManager>  m_networkManager;
};

#endif // MATTERMOSTQT_H
