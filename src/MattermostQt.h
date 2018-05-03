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
		Channels,
		User
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

	struct UserContainer
	{
		UserContainer() {
			m_update_at = 0;
		}

		UserContainer(QJsonObject object);
		//"id": "string",
		QString m_id;
		//"create_at": 0,
		//"update_at": 0,
		qlonglong m_update_at;
		//"delete_at": 0,
		//"username": "string",
		QString m_username;
		//"first_name": "string",
		QString m_first_name;
		//"last_name": "string",
		QString m_last_name;
		//"nickname": "string",
		QString m_nickname;
		//"email": "string",
		//"email_verified": true,
		//"auth_service": "string",
		//"roles": "string",
		//"locale": "string",
		//"notify_props": {
		//"email": "string",
		//"push": "string",
		//"desktop": "string",
		//"desktop_sound": "string",
		//"mention_keys": "string",
		//"channel": "string",
		//"first_name": "string"
		//},
		//"props": { },
		//"last_password_update": 0,
		qlonglong m_last_password_update;
		//"last_picture_update": 0,
		qlonglong m_last_picture_update;
		//"failed_attempts": 0,
		//"mfa_active": true

		int m_self_index;
	};

	typedef QSharedPointer<UserContainer> UserPtr;

	struct ChannelContainer
	{
		ChannelContainer()
		{
			m_teamId = -1;
			m_serverId = -1;
			m_self_index = -1;
			m_update_at = 0;
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
		int     m_self_index;   /**< self index in vector */
	};
	typedef QSharedPointer<ChannelContainer> ChannelPtr;

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
		int        m_self_index;   /**< self index in vector */

		QVector<ChannelPtr> m_public_channels;
		QVector<ChannelPtr> m_private_channels;
		QVector<ChannelPtr> m_direct_channels;
	};
	typedef QSharedPointer<TeamContainer> TeamPtr;

	struct ServerContainer
	{
	public:
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
		QString                     m_user_id;/**< user id */
		int                         m_selfId; /**< server index in QVector */
		QList<TeamPtr>              m_teams; /**< allowed teams */

		QString  m_config_path; /**< local config path */
	};
	typedef QSharedPointer<ServerContainer> ServerPtr;

public:
	MattermostQt();

	~MattermostQt();

	Q_INVOKABLE void post_login(QString server, QString login, QString password, bool trustCertificate = false, int api = 4);
	void get_login(ServerPtr sc);
	Q_INVOKABLE void get_teams(int serverId);
	Q_INVOKABLE void get_public_channels(int serverId, QString teamId);
	Q_INVOKABLE void get_user_image(int serverId, QString userId);
	Q_INVOKABLE void get_user_info(int serverId, QString userId);

	bool save_settings();
	bool load_settings();

Q_SIGNALS:
	void serverConnected(int id);
	void connectionError(int code, QString message);
	void teamAdded(TeamPtr team);
	void channelAdded(ChannelPtr channel);

protected:
	/**
	 * @brief prepare_direct_channel
	 * when we have direct channel? we need get user id, login and profile
	 * image before we send to ChannelsModel
	 * @param channel
	 */
	void prepare_direct_channel(int server_index, int tem_index, int channel_index);

	void websocket_connect(ServerPtr server);

	bool reply_login(QNetworkReply *reply);
	void reply_get_teams(QNetworkReply *reply);
	void reply_get_public_channels(QNetworkReply *reply);
	void reply_get_user(QNetworkReply *reply);
	void reply_error(QNetworkReply *reply);

protected Q_SLOTS:
	void replyFinished(QNetworkReply *reply);
	void replySSLErrors(QNetworkReply *reply, QList<QSslError> errors);

	void onWebSocketConnected();
	void onWebSocketSslError(QList<QSslError> errors);
	void onWebSocketError(QAbstractSocket::SocketError error);
protected:
	QMap<int, ServerPtr>    m_server;
	QSharedPointer<QNetworkAccessManager>  m_networkManager;

	QString m_settings_path;
//	QTimer  m_settings_timer;

	/** channels, need update before put to model */
//	QList<ChannelContainer>   m_stackedChannels;
};

#endif // MATTERMOSTQT_H
