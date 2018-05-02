#ifndef MATTERMOSTQT_H
#define MATTERMOSTQT_H

#include <QObject>
#include <QString>
#include <QUrl>
#include <QMap>
#include <QNetworkAccessManager>
//#include "libs/qtwebsockets/include/QtWebSockets/qwebsocket.h"
#include <QtWebSockets/QWebSocket>

class MattermostQt : public QObject
{
	Q_OBJECT
public:
	enum ReplyType : int {
		Login,
		Teams,
		Team
	};

	enum ConnectionError {
		WrongPassword,
		SslError
	};

	enum ChannelType {
		Open,   // "O"
		Private,// "P"
		Direct  // "D"
	};

	struct ChannelContainer
	{
		ChannelContainer()
		{

		}

		ChannelContainer(QJsonObject &object);

		QString m_id;
//		"create_at": 0,
		qlonglong m_update_at;
//		"delete_at": 0,
		QString m_team_id;
		QString m_type;
		QString m_display_name;
		QString m_name;
		QString m_header;
		QString m_purpose;
		qlonglong m_last_post_at;
		qlonglong m_total_msg_count;
		qlonglong m_extra_update_at;
		QString m_creator_id;
		int     m_teamId;   /**< team index in QVector */
		int     m_serverId; /**< server index in QVector */
		int     m_selfId;   /**< self index in vector */
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
		int        m_serverId; /**< server index in QVector */
		int        m_selfId;   /**< self index in vector */

		QVector<ChannelContainer> m_public_channels;
		QVector<ChannelContainer> m_private_channels;
		QVector<ChannelContainer> m_direct_channels;
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
		QString                     m_cookie;/**< cookie */
		bool                        m_trustCertificate; /**< trust self signed certificate */
		QSslConfiguration           m_cert;  /**< server certificate */
		QSharedPointer<QWebSocket>  m_socket;/**< websocket connection */
		QList<TeamContainer>        m_teams; /**< allowed teams */
		QString                     m_user_id;/**< user id */
		int                         m_selfId;    /**< server index in QVector */
	};

public:
	MattermostQt();

	~MattermostQt();

	Q_INVOKABLE void post_login(QString server, QString login, QString password, bool trustCertificate = false, int api = 4);
	Q_INVOKABLE void get_teams(int serverId);
	Q_INVOKABLE void get_public_channels(int serverId, QString teamId);

	void saveSettings();

Q_SIGNALS:
	void serverConnected(int id);
	void connectionError(int code, QString message);
	void teamAdded(MattermostQt::TeamContainer team);
	void channelAdded(MattermostQt::ChannelContainer channel);

protected:
	void websocket_connect(ServerContainer &server);

	bool reply_login(QNetworkReply *reply);
	void reply_get_teams(QNetworkReply *reply);
	void reply_get_public_channels(QNetworkReply *reply);
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

	/** channels, need update before put to model */
	QList<ChannelContainer>   m_stackedChannels;
};

#endif // MATTERMOSTQT_H
