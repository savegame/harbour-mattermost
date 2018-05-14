#ifndef MATTERMOSTQT_H
#define MATTERMOSTQT_H

#include <QObject>
#include <QString>
#include <QSize>
#include <QUrl>
#include <QMap>
#include <QTimer>
#include <QNetworkAccessManager>
#include "libs/qtwebsockets/include/QtWebSockets/qwebsocket.h"
//#include <QtWebSockets/QWebSocket>

class MattermostQt : public QObject
{
	Q_OBJECT

	friend class MessagesModel;
public:
	enum ReplyType : int {
		Login,
		Teams,
		Channels,
		rt_get_user_info,
		rt_get_user_image,
		rt_get_team,
		rt_get_teams_unread,
		rt_get_posts,
		rt_get_posts_since,
		rt_get_posts_before,
		rt_get_posts_after,
		rt_get_file_thumbnail,
		rt_get_file,
		rt_get_file_info,
		rt_post_send_message
	};

	enum ConnectionError {
		WrongPassword,
		SslError
	};

	enum FileType {
		FileUnknown,
		FileDocument,
		FileImage,
		FileAnimatedImage,
	};
	Q_ENUMS(FileType)

	enum ChannelType : int {
		ChannelPublic,   // "O"
		ChannelPrivate,// "P"
		ChannelDirect,  // "D"
		ChannelTypeCount
	};
	Q_ENUMS(ChannelType)

	enum MessageType {
		MessageSystem,// system message
		MessageOther ,// when ither posted message
		MessageMine,  // when message posted by myself
		MessageTypeCount
	};
	Q_ENUMS(MessageType)

	enum ServerState : int {
		ServerConnected = QAbstractSocket::ConnectedState,
		ServerConnecting = QAbstractSocket::ConnectingState,
		ServerUnconnected = QAbstractSocket::UnconnectedState
	};
	Q_ENUMS(ServerState)

	/**
	 * @brief The FileContainer struct
	 * all files list stored in serverptr
	 */
	struct FileContainer {
		FileContainer() {}

		FileContainer(QJsonObject object);

		// file info
		QString m_id;
		QString m_post_id; // message id
		QString m_user_id;
		bool m_has_preview_image;
		QString m_name;
		QString m_extension;
		QString m_mime_type;
		qlonglong m_file_size;
		FileType m_file_type;
		// if it image
		QSize   m_image_size;
		QString m_thumb_path;

		int m_server_index;
		int m_team_index;
		int m_channel_type;
		int m_channel_index;
		int m_message_index; // post index
		int m_self_index; // in message files list
		int m_self_sc_index; // in server files list
//		MessagePtr m_message;// test
	};
	typedef QSharedPointer<FileContainer> FilePtr;

	struct MessageContainer {
		MessageContainer()
		{}

		MessageContainer(QJsonObject object);

		QString          m_message;
//		QVector<FilePtr> m_file;
		QVector<QString> m_filenames;
		QVector<QString> m_file_ids;
		QString          m_id;
		QString          m_channel_id;
		QString          m_type_string;
		MessageType      m_type;
		QString          m_user_id;

		// inside types
		QVector<FilePtr>     m_file;
		ChannelType      m_channel_type;
		int              m_user_index;
		int              m_server_index;
		int              m_team_index;
		int              m_channel_index;
		int              m_self_index;

//		ChannelPtr       m_channel;// test
	};
	typedef QSharedPointer<MessageContainer> MessagePtr;

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
		QString m_image_path;
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

		bool save_json(QString server_dir_path) const;
		bool load_json(QString server_dir_path);

		QString m_id;
//		"create_at": 0,
		qlonglong m_update_at;
//		"delete_at": 0,
		QString m_team_id;
		ChannelType m_type;
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

		QVector<MessagePtr> m_message;
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
		/**
		 * @brief save_json
		 * @param server_dir_path   directory where all teams data stored
		 * ~/.config/{mattermost_config_dir}/{server_dir}
		 * @return
		 */
		bool save_json(QString server_dir_path) const;
		bool load_json(QString server_dir_path);

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
		QSslConfiguration           m_cert;  /**< server certificate */
		QSharedPointer<QWebSocket>  m_socket;/**< websocket connection */
		QString                     m_user_id;/**< user id */
		int                         m_self_index; /**< server index in QVector */
		QVector<TeamPtr>            m_teams; /**< allowed teams */
		int                         m_state; /**< server state (from WebSocket) */
		QString                     m_config_path; /**< local config path */
		QVector<UserPtr>            m_user;/**< list of users by theirs id's */
		QVector<ChannelPtr>         m_direct_channels; /**< direct channels */
		QString                     m_display_name; /**< custom server name */
		bool                        m_trust_cert; /**< trust self signed certificate */
		QString                     m_ca_cert_path;
		QString                     m_cert_path;
		QVector<FilePtr>            m_file;
	};
	typedef QSharedPointer<ServerContainer> ServerPtr;

public:
	MattermostQt();

	~MattermostQt();

	Q_INVOKABLE int get_server_state(int server_index);
	Q_INVOKABLE int get_server_count() const;
	Q_INVOKABLE QString get_server_name(int server_index) const;

	Q_INVOKABLE void post_login(QString server, QString login, QString password,
	                            int api = 4,QString display_name = QString("Mattermost Server"),
	                            bool trustCertificate = false, QString ca_cert_path = QString(), QString cert_path = QString());
	void get_login(ServerPtr sc);
	Q_INVOKABLE void get_teams(int server_index);
	Q_INVOKABLE void get_public_channels(int server_index, QString team_id);
//	void get_team(int server_index, QString team_id);
	void get_team(int server_index, int team_index);
	void get_file_thumbnail(int server_index, int file_sc_index);
//	void get_file_thumbnail(int server_index, int team_index, int channel_type,
//	                        int channel_index, int message_index, QString file_id);
	Q_INVOKABLE void get_file_info(int server_index, int team_index, int channel_type,
	                   int channel_index, int message_index, QString file_id);
	Q_INVOKABLE void post_send_message(QString message, int server_index, int team_index, int channel_type,
	                                   int channel_index);
	Q_INVOKABLE void get_user_image(int server_index, int user_index);
	Q_INVOKABLE void get_user_info(int server_index, QString userId,  int team_index = -1);
	Q_INVOKABLE void get_teams_unread(int server_index);
//	Q_INVOKABLE void get_posts(int server_index, int team_index, QString channel_id);
	Q_INVOKABLE void get_posts(int server_index, int team_index, int channel_index, int channel_type);
	Q_INVOKABLE void get_posts_before(int server_index, int team_index, int channel_index, int channel_type);

	bool save_settings();
	bool load_settings();

Q_SIGNALS:
	void serverConnected(int server_index);
	void serverStateChanged(int server_index, int state);
	void connectionError(int code, QString message);
	void teamAdded(TeamPtr team);
	void teamsExists(const QVector<MattermostQt::TeamPtr> &teams);
	void channelsList(QList<ChannelPtr> list);
	void channelAdded(ChannelPtr channel);
//	void updateChannel()
	void teamUnread(QString team_id, int msg, int mention);
	void messagesAdded(ChannelPtr channel);
	void messagesAddedBefore(ChannelPtr channel, int count);
	void messageAdded(QList<MessagePtr> messages);
	void messageUpdated(QList<MessagePtr> messages);
	void userUpdated(UserPtr user);
protected:
	/**
	 * @brief prepare_direct_channel
	 * when we have direct channel? we need get user id, login and profile
	 * image before we send to ChannelsModel
	 * @param channel
	 */
	void prepare_direct_channel(int server_index, int team_index, int channel_index);
	/**
	 * @brief prepare_user_index
	 * @param server_index
	 * @param message
	 */
	void prepare_user_index(int server_index, MessagePtr message);

	void get_teams_unread(ServerPtr server);

	void websocket_connect(ServerPtr server);

	bool reply_login(QNetworkReply *reply);
	void reply_get_teams(QNetworkReply *reply);
	void reply_get_team(QNetworkReply *reply);
	void reply_get_teams_unread(QNetworkReply *reply);
	void reply_get_posts(QNetworkReply *reply);
	void reply_get_posts_before(QNetworkReply *reply);
	void reply_get_public_channels(QNetworkReply *reply);
	void reply_get_user_info(QNetworkReply *reply);
	void reply_error(QNetworkReply *reply);
	void reply_get_file_thumbnail(QNetworkReply *reply);
	void reply_get_file_info(QNetworkReply *reply);
	void reply_get_user_image(QNetworkReply *reply);
	void reply_post_send_message(QNetworkReply *reply);

	void event_posted(ServerPtr sc, QJsonObject data);
	void event_post_edited(ServerPtr sc, QJsonObject data);
	void event_post_deleted(ServerPtr sc, QJsonObject data);

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
	void slot_recconect_servers();
protected:
	QVector<ServerPtr>    m_server;
	QSharedPointer<QNetworkAccessManager>  m_networkManager;

	QString m_settings_path;
//	QTimer  m_settings_timer;

	int    m_update_server_timeout;
	QTimer m_reconnect_server;

	/** channels, need update before put to model */
//	QList<ChannelContainer>   m_stackedChannels;
};

#endif // MATTERMOSTQT_H
