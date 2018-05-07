#ifndef MATTERMOSTQT_H
#define MATTERMOSTQT_H

#include <QObject>
#include <QString>
#include <QUrl>
#include <QMap>
#include <QTimer>
#include <QNetworkAccessManager>
#include "libs/qtwebsockets/include/QtWebSockets/qwebsocket.h"
//#include <QtWebSockets/QWebSocket>

class MattermostQt : public QObject
{
	Q_OBJECT

//	Q_PROPERTY(int serverState READ get_server_state NOTIFY serverStateChanged)
//	Q_PROPERTY(type name READ name WRITE setName NOTIFY nameChanged)
public:
	enum ReplyType : int {
		Login,
		Teams,
		Channels,
		get_user,
		rt_get_team,
		rt_get_teams_unread
	};

	enum ConnectionError {
		WrongPassword,
		SslError
	};

	enum ChannelType : int {
		ChannelOpen,   // "O"
		ChannelPrivate,// "P"
		ChannelDirect  // "D"
	};

	enum ServerState : int {
		ServerConnected = QAbstractSocket::ConnectedState,
		ServerConnecting = QAbstractSocket::ConnectingState,
		ServerUnconnected = QAbstractSocket::UnconnectedState
	};
	Q_ENUMS(ServerState)

	struct UserContainer
	{
		UserContainer() {
			m_update_at = 0;
		}

		UserContainer(QJsonObject object);
		QString m_id;
//		qlonglong m_create_at;
		qlonglong m_update_at;
		//"delete_at": 0,
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
		QString m_locale;
		//"notify_props": {
		//	"email": "string",
		//	"push": "string",
		//	"desktop": "string",
		//	"desktop_sound": "string",
		//	"mention_keys": "string",
		//	"channel": "string",
		//	"first_name": "string"
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
			m_team_index = -1;
			m_server_index = -1;
			m_self_index = -1;
			m_update_at = 0;
			m_dc_user_index = -1;
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
		int     m_team_index;   /**< team index in QVector */
		int     m_server_index; /**< server index in QVector */
		int     m_self_index;   /**< self index in vector */

		// direct channel data
		int m_dc_user_index; /**< if it direct channel, is index*/
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
		int        m_server_index; /**< server index in QVector */
		int        m_self_index;   /**< self index in vector */

		QVector<ChannelPtr> m_public_channels;
		QVector<ChannelPtr> m_private_channels;
		QVector<ChannelPtr> m_direct_channels;
	};
	typedef QSharedPointer<TeamContainer> TeamPtr;

	struct ServerContainer
	{
	public:
		ServerContainer() : m_api(4), m_trust_cert(false) {}

		ServerContainer(QString url, QString token, int api)
		{
			m_url = url;
			m_api = api;
			m_token = token;
			m_trust_cert = false;
		}

		~ServerContainer() ;

		int get_team_index(QString team_id);

		QString                     m_url;   /**< server URL */
		int                         m_api;   /**< server API version */
		QString                     m_token; /**< server access token*/
		QString                     m_cookie;/**< cookie if needed */
		bool                        m_trust_cert; /**< trust self signed certificate */
		QSslConfiguration           m_cert;  /**< server certificate */
		QSharedPointer<QWebSocket>  m_socket;/**< websocket connection */
		QString                     m_user_id;/**< user id */
		int                         m_self_index; /**< server index in QVector */
		QList<TeamPtr>              m_teams; /**< allowed teams */
		int                         m_state; /**< server state (from WebSocket) */
		QString                     m_config_path; /**< local config path */
		QVector<UserPtr>            m_user;/**< list of users by theirs id's */
		QString                     m_display_name; /**< custom server name */
	};
	typedef QSharedPointer<ServerContainer> ServerPtr;

public:
	MattermostQt();

	~MattermostQt();

	Q_INVOKABLE int get_server_state(int server_index);
	Q_INVOKABLE int get_server_count() const;

	Q_INVOKABLE void post_login(QString server, QString login, QString password,
	                            bool trustCertificate = false, int api = 4,
	                            QString display_name = QString("Mattermost Server"));
	void get_login(ServerPtr sc);
	Q_INVOKABLE void get_teams(int server_index);
	Q_INVOKABLE void get_public_channels(int server_index, QString team_id);
//	void get_team(int server_index, QString team_id);
	void get_team(int server_index, int team_index);
	Q_INVOKABLE void get_user_image(int server_index, QString user_id);
	Q_INVOKABLE void get_user_info(int server_index, QString userId,  int team_index = -1);
	Q_INVOKABLE void get_teams_unread(int server_index);

	bool save_settings();
	bool load_settings();

Q_SIGNALS:
	void serverConnected(int server_index);
	void serverStateChanged(int server_index, int state);
	void connectionError(int code, QString message);
	void teamAdded(TeamPtr team);
	void channelsList(QList<ChannelPtr> list);
	void channelAdded(ChannelPtr channel);
//	void updateChannel()
	void teamUnread(QString team_id, int msg, int mention);

protected:
	/**
	 * @brief prepare_direct_channel
	 * when we have direct channel? we need get user id, login and profile
	 * image before we send to ChannelsModel
	 * @param channel
	 */
	void prepare_direct_channel(int server_index, int team_index, int channel_index);

	void get_teams_unread(ServerPtr server);

	void websocket_connect(ServerPtr server);

	bool reply_login(QNetworkReply *reply);
	void reply_get_teams(QNetworkReply *reply);
	void reply_get_team(QNetworkReply *reply);
	void reply_get_teams_unread(QNetworkReply *reply);
	void reply_get_public_channels(QNetworkReply *reply);
	void reply_get_user(QNetworkReply *reply);
	void reply_error(QNetworkReply *reply);

protected Q_SLOTS:
	void replyFinished(QNetworkReply *reply);
	void replySSLErrors(QNetworkReply *reply, QList<QSslError> errors);

	void onWebSocketConnected();
	void onWebSocketSslError(QList<QSslError> errors);
	void onWebSocketError(QAbstractSocket::SocketError error);
	void onWebSocketStateChanged(QAbstractSocket::SocketState state);
//	void onWebSocketTextFrameReceived(const QString &frame, bool isLastFrame);
	void onWebSocketTextMessageReceived(const QString &message);
	/** slot for QTimer */
	void slot_get_teams_unread();
protected:
	QVector<ServerPtr>    m_server;
	QSharedPointer<QNetworkAccessManager>  m_networkManager;

	QString m_settings_path;
//	QTimer  m_settings_timer;

	int    m_update_server_timeout;
	QTimer m_update_server;

	/** channels, need update before put to model */
//	QList<ChannelContainer>   m_stackedChannels;
};

#endif // MATTERMOSTQT_H
