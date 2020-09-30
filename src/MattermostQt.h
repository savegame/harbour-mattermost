#ifndef MATTERMOSTQT_H
#define MATTERMOSTQT_H

#include <QObject>
#include <QString>
#include <QSize>
#include <QUrl>
#include <QMap>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QtWebSockets/qwebsocket.h>
#include "MarkdownParser.h"
#include <MattermostQt_gobal.h>
#include <QtQuick>
//#include <QtWebSockets/QWebSocket>
//#include <QWebSocket>

// #define PRELOAD_FILE_INFOS //use old style of loading messages with 

#define MM_DEFAULT_PER_PAGE 50

class SettingsContainer;

class MattermostQt : public QObject
{
	Q_OBJECT

	friend class MessagesModel;
	friend class ChannelsModel;
//	friend QObject* MattermostQt_singletontype_provider(QQmlEngine *engine, QJSEngine *scriptEngine);
public:
	static MattermostQt *getInstance();
public:
	enum ReplyType : int {
		rt_login = 0,
		rt_get_teams,
		rt_get_public_channels,
		rt_get_team_public_channels,
		rt_get_team_members,
		rt_get_team_statistics,
		rt_get_channel,
		rt_post_channel_view,
		rt_post_add_user_to_channel,
		rt_delete_user_from_channel,
		rt_post_create_channel,
		rt_get_user_info,
		rt_get_user_image,
		rt_post_users_status,
		rt_get_team,
		rt_get_teams_unread,
		rt_get_posts,
		rt_get_post,
		rt_get_posts_since,
		rt_get_posts_before,
		rt_get_posts_after,
		rt_get_file_thumbnail,
		rt_get_file_preview,
		rt_get_file,
		rt_post_file_upload,
		rt_get_file_info,
		rt_post_send_message,
		rt_delete_message,
		rt_put_message_edit,
		rt_get_channel_unread,
		rt_get_channel_members,
		rt_get_channel_statistics,
		rt_get_team_icon,
		rt_post_create_reaction,
		rt_delete_reaction,
		//======================
		ReplyTypeCount
	};
	Q_ENUM(ReplyType)

	enum EventType : int {
		et_noevent = 0,
		et_posted,
		et_post_edited,
		et_post_deleted,
		et_status_change,
		et_typing,
		et_channel_viewed,
		et_reaction_added,
		et_reaction_removed,
		et_channel_created,
		et_user_added,
		et_user_removed,
		//=======================
		EventTypeCount
	};
	Q_ENUM(EventType)

	enum ConnectionError {
		WrongPassword,
		SslError,
		SessionExpired,
		WrongPasswordOrEmail,
		UnknownError
	};
	Q_ENUM(ConnectionError)

	enum FileType {
		FileUnknown,
		FileDocument,
		FileImage,
		FileAnimatedImage,
	};
	Q_ENUM(FileType)

	enum FileStatus {
		FileDownloaded,
		FileDownloading,
		FileRemote, //that mean not downloaded
		FileUninitialized,
		FileRequested,
	};
	Q_ENUM(FileStatus)

	enum ChannelType : int {
		ChannelPublic,   // "O"
		ChannelPrivate,// "P"
		ChannelDirect,  // "D"
		ChannelTypeCount
	};
	Q_ENUM(ChannelType)

	enum MessageOwner {
		MessageSystem,// system message
		MessageOther ,// when ither posted message
		MessageMine,  // when message posted by myself
		MessageDate,  // date of messages group
		MessageTypeCount
	};
	Q_ENUM(MessageOwner)

	enum ServerState : int {
		ServerConnected = QAbstractSocket::ConnectedState,
		ServerConnecting = QAbstractSocket::ConnectingState,
		ServerUnconnected = QAbstractSocket::UnconnectedState,
		ServerLogin // when server just start logging in
	};
	Q_ENUMS(ServerState)

	enum UserStatus : int {
		UserNoStatus,
		UserOnline,
		UserAway,
		UserOffline,
		UserDnd
	};
	Q_ENUM(UserStatus)

	enum UserDataRole {
		UserStatusRole = Qt::UserRole + 100,
		UserImageRole,
		UserNameRole,
		UserLastActivityRole
	};
	Q_ENUM(UserDataRole)

	/**
	 * @brief The UserSystemRole enum
	 * user mattermost roles
	 */
	enum UserSystemRole {
		SystemUser  = 0,
		SystemAdmin,
		UserSystemRolesCount
	};
	Q_ENUM(UserSystemRole)

	enum ApplicationStatus : int {
		AppActive,
		AppInactive
	};
	Q_ENUM(ApplicationStatus)

	/**
	 * @brief The FileContainer struct
	 * all files list stored in serverptr
	 */
	struct FileContainer {
		FileContainer() noexcept;

		FileContainer(QJsonObject object) noexcept;

		bool save_json(QString server_data_path) const;
		bool load_json(QString server_data_path);

		void parse_from_json(QJsonObject object);

		QString filename() const { return m_id + QLatin1String("_") + m_name; }
		QString filedir(QString cache_path) const
		{
			return cache_path + QLatin1String("/files/") + m_id;
		}

		bool is_file_in_cache(QString cache_path) {
//			if(m_is_in_cache == 2) {
				m_is_in_cache = (m_file_path.indexOf(filedir(cache_path)) == 0)?1:0;
//			}
			return m_is_in_cache == 1;
		}

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
		FileStatus m_file_status;

		QString m_file_path;
		// if it image
		QSize   m_image_size;
		QString m_thumb_path;
		QString m_preview_path;
		QSizeF  m_item_size; //computed item size for list view
		int     m_contentwidth;
		// if it document

		int m_server_index;
		int m_team_index;
		int m_channel_type;
		int m_channel_index;
		int m_message_index; // post index
		int m_self_index; // in message files list
		int m_self_sc_index; // in server files list
//		MessagePtr m_message;// test
		char m_is_in_cache;
	};
	typedef QSharedPointer<FileContainer> FilePtr;


	struct ReactionContainer {
		QString m_path;
		QString m_emoji;
		QVector<QString> m_user_id;
		/// if we too add this reaction
		bool    m_mine_emoji = false;
	};

	struct MessageContainer {
		typedef QSharedPointer<MessageContainer> Ptr;

		MessageContainer() noexcept;

		MessageContainer(QJsonObject object, const QString &user_id);

		/** update files states (indexes in MAttermostQt structure) */
		bool updateFiles(MattermostQt *mattermost = nullptr);
		bool updateRootMessage(MattermostQt *mattermost);
		bool addReaction(ReactionContainer &reaction);
		bool removeReaction(const ReactionContainer &reaction);

		QString          m_message;
		QString          m_formated_message;
//		QVector<FilePtr> m_file;
		QVector<QString> m_filenames;
		QVector<QString> m_file_ids;
		QString          m_id;
		QString          m_channel_id;
		QString          m_type_string;
		MessageOwner     m_type;
		QString          m_user_id;
		QString          m_root_id; /**< id of root message of thread*/
		QString          m_parent_id;
		QString          m_original_id;
		qlonglong        m_create_at;
		qlonglong        m_update_at;
		qlonglong        m_delete_at;
		// reactions
//		QVector<QString>  m_reactions_paths; // for simple view only
//		QVector<int>      m_reactions_count;
		QVector<ReactionContainer> m_reactions;

		// inside types
		QVector<FilePtr>     m_file;
		ChannelType      m_channel_type;
		int              m_user_index;  // index in server users
		int              m_server_index;
		int              m_team_index;
		int              m_channel_index;
		int              m_self_index;
		qreal            m_item_height;
		// data for reply message
		QString          m_root_message;
		QString          m_root_user_name;
		int              m_root_user_index;
		Ptr              m_root_ptr; // it can be emty, if message not downloaded
		QList<Ptr>       m_thread_messages; // answers to this message (for dynamic update it, when message is edited)

		/// special parameters, delete if not use
//		ChannelPtr       m_channel;// test
		bool             m_is_files_info_requested; // is files already requested from server
		qlonglong        m_request_time;  // request time, for repeat if it need
		FilePtr   fileAt(int file_index);
	};
	typedef MessageContainer::Ptr MessagePtr;

	struct UserContainer
	{
		UserContainer() noexcept {
			m_update_at = 0;
			m_last_activity_at = 0;
			memset(m_roles, false, UserSystemRolesCount * sizeof(bool) );
		}

		UserContainer(QJsonObject object);

		bool operator ==(const UserContainer &other) const {
			return other.m_id == m_id;
		}

		bool operator !=(const UserContainer &other) const {
			return other.m_id != m_id;
		}

		bool operator >(const UserContainer &other) const {
			return other.m_id > m_id;
		}

		bool operator <(const UserContainer &other) const {
			return other.m_id < m_id;
		}

		QString m_id;
//		qlonglong m_create_at;
		qlonglong m_update_at = 0;
		qlonglong m_delete_at = 0;
		//"delete_at": 0,
		QString m_username;
		QString m_first_name;
		QString m_last_name;
		QString m_nickname;
		//"email": "string",
		//"email_verified": true,
		//"auth_service": "string",
		//"roles": "string",
		bool m_roles[UserSystemRolesCount]; //

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
		qlonglong m_last_password_update;
		qlonglong m_last_picture_update;
		qlonglong m_last_activity_at;
		//"failed_attempts": 0,
		//"mfa_active": true
		bool      m_typing = false;

		int m_self_index;
		UserStatus  m_status;
		QString m_image_path;
	};
	typedef QSharedPointer<UserContainer> UserPtr;

	struct ChannelContainer
	{
		ChannelContainer() noexcept
		{
			m_team_index = -1;
			m_server_index = -1;
			m_self_index = -1;
			m_update_at = 0;
			m_dc_user_index = -1;
			m_total_msg_count = 0;
		}

		ChannelContainer(QJsonObject &object) noexcept ;

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
		int     m_team_index    = -1; /**< team index in QVector */
		int     m_server_index  = -1; /**< server index in QVector */
		int     m_self_index    = -1; /**< self index in vector */
		int     m_msg_unread    =  0; /**< count of unread messages **/
		int     m_mention_count =  0; /**< count of mention in channel **/
		int     m_member_count  = -1; /**< channel's members count */
		QList<UserPtr> m_user_typing;  /**< when we have event user_typing */

		// direct channel data
		int m_dc_user_index; /**< if it direct channel, is index*/

		QVector<MessagePtr> m_message;
		QVector<MessagePtr> m_unorder_messages; /** not ordered messages z*/

		MessagePtr messageAt(int message_index);
	};
	typedef QSharedPointer<ChannelContainer> ChannelPtr;

	struct TeamContainer
	{
		TeamContainer() noexcept
		{
			m_create_at = 0;
			m_update_at = 0;
			m_delete_at = 0;
			m_unread_messages = 0;
			m_unread_mentions = 0;
		}

		TeamContainer(QJsonObject &object) noexcept ;
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
		qlonglong  m_create_at = 0;
		qlonglong  m_update_at = 0;
		qlonglong  m_delete_at = 0;
		int        m_server_index; /**< server index in QVector */
		int        m_self_index;   /**< self index in vector */
		QString    m_image_path;
		
		int        m_total_member_count = -1;
		int        m_active_member_count = -1;

		// unread data
		int  m_unread_messages = 0;
		int  m_unread_mentions = 0;

		QVector<ChannelPtr> m_public_channels;
		QVector<ChannelPtr> m_private_channels;
	};
	typedef QSharedPointer<TeamContainer> TeamPtr;

//	struct UnattachedMessageContainer {
//		UnattachedMessageContainer() noexcept {}
//		QString     m_team_id;
//		MessagePtr  m_message;
//	};
//	typedef QSharedPointer<UnattachedMessageContainer> UMessagePtr;

	struct ServerContainer
	{
	public:
		ServerContainer() : m_api(4), m_trust_cert(false), m_enabled(true) {}

		ServerContainer(QString url, QString token, int api);

		~ServerContainer() ;

		int get_team_index(QString team_id);

		QString                     m_url;   /**< server URL */
		int                         m_api;   /**< server API version */
		QString                     m_token; /**< server access token*/
		QString                     m_bearer;/**< cached authorization bearer header */
		QString                     m_cookie;/**< cookie if needed */
		QSslConfiguration           m_cert;  /**< server certificate */
		QSharedPointer<QWebSocket>  m_socket;/**< websocket connection */
		QString                     m_user_id;/**< user id */
		UserPtr                     m_current_user;/**< current user */
		int                         m_self_index; /**< server index in QVector */
		QVector<TeamPtr>            m_teams; /**< allowed teams */
		int                         m_state; /**< server state (from WebSocket) */
		QString                     m_data_path; /**< local config path */
		QString                     m_cache_path;
		QVector<UserPtr>            m_user;/**< list of users by theirs id's */
		QVector<ChannelPtr>         m_direct_channels; /**< direct channels */
		QString                     m_display_name; /**< custom server name */
		bool                        m_trust_cert; /**< trust self signed certificate */
		QString                     m_ca_cert_path;
		QString                     m_cert_path;
		QVector<FilePtr>            m_file;
		QList<FilePtr>              m_unattached_file; /**< uploaded, but not sended files */
		QList<FilePtr>              m_sended_files; /**<  */
		QList<MessagePtr>           m_nouser_messages;/**< messages without user */
		bool                        m_enabled; /**< is server is enabled */
		QHash<QString,ChannelPtr>   m_channels_hash; /**< yfshed list of channels*/

		QTimer                      m_ping_timer;
		QVector<QString>            m_requested_users; /** requested users id's */

		QString                     m_auth_data;
		QString                     m_auth_service;
	};
	typedef QSharedPointer<ServerContainer> ServerPtr;

	typedef void (MattermostQt::*event_func)(ServerPtr sc, QJsonObject data);
	void (MattermostQt::*m_event_func[EventTypeCount])(ServerPtr sc, QJsonObject data);
	typedef void (MattermostQt::*reply_func)(QNetworkReply* reply);
	void (MattermostQt::*m_reply_func[ReplyTypeCount])(QNetworkReply* reply);
	void bind_reply_functions();
	void bind_event_functions();

	Q_PROPERTY(int messageUnread READ messageUnread NOTIFY messageUnreadChanged)
	Q_PROPERTY(ApplicationStatus applicationStatus READ getApplicationStatus WRITE setApplicationStatus NOTIFY onApplciationStatusChanged)
public:
	MattermostQt(QObject *parent = nullptr);

	~MattermostQt();
	
	/**
	 * @brief replaceMdParser 
	 * @param parser
	 */
	void replaceMdParser(MarkdownParser *parser);

	ApplicationStatus getApplicationStatus() const;
	void setApplicationStatus(ApplicationStatus status);

	Q_INVOKABLE QString getVersion() const;
	Q_INVOKABLE QString emojiPath() const;
	// Server helper functions
	Q_INVOKABLE int     get_server_state(int server_index);
	Q_INVOKABLE void    force_server_recconect();
	Q_INVOKABLE int     get_server_count() const;
	Q_INVOKABLE QString get_server_name(int server_index) const;
	Q_INVOKABLE QString get_server_url(int server_index) const;
	Q_INVOKABLE bool    get_server_trust_certificate(int server_index) const;
	Q_INVOKABLE QString get_server_cert_path(int server_index) const;
	Q_INVOKABLE QString get_server_ca_cert_path(int server_index) const;
	Q_INVOKABLE void    set_server_enabled(int server_index, const bool enabled);


	Q_INVOKABLE void post_login(QString server, QString login, QString password,
	                            int api = 4,QString display_name = QString("Mattermost Server"),
	                            bool trustCertificate = false, QString ca_cert_path = QString(), QString cert_path = QString());
	Q_INVOKABLE void post_login_by_token(QString url, QString token,
	                                     int api = 4,QString display_name = QString("Mattermost Server"),
	                                     bool trustCertificate = false, QString ca_cert_path = QString(), QString cert_path = QString());
	void get_login(ServerPtr sc);
	Q_INVOKABLE void get_teams(int server_index);
	Q_INVOKABLE void get_team_icon(int server_index, int team_index);
	Q_INVOKABLE void get_team_public_channels(int server_index, QString team_id);
	Q_INVOKABLE void get_team(int server_index, int team_index);
    Q_INVOKABLE void get_team_members(int server_index, int team_index, int page = 0, int per_page = MM_DEFAULT_PER_PAGE);
	Q_INVOKABLE void get_team_statistics(int server_index, int team_index);
	Q_INVOKABLE void get_public_channels(int server_index, QString team_id);
	Q_INVOKABLE void get_channel(int server_index, QString channel_id);
	Q_INVOKABLE void get_channel(int server_index, int team_index, int channel_type, int channel_index);
	void get_channel_unread(ChannelPtr channel);
	Q_INVOKABLE void get_channel_unread(int server_index, int team_index,
	                                    int channel_type, int channel_index);
	Q_INVOKABLE void get_channel_members(int server_index, int team_index,
	                                    int channel_type, int channel_index, 
	                                     int page = 0, int per_page = MM_DEFAULT_PER_PAGE);
	Q_INVOKABLE void get_channel_statistics(int server_index, int team_index,
	                                        int channel_type, int channel_index);
	
//	void get_team(int server_index, QString team_id);
	void get_file_thumbnail(int server_index, int file_sc_index);
	void get_file_preview(int server_index, int file_sc_index);
//	void get_file_thumbnail(int server_index, int team_index, int channel_type,
//	                        int channel_index, int message_index, QString file_id);
	Q_INVOKABLE void get_file_info(int server_index, int team_index, int channel_type,
	                   int channel_index, int message_index, QString file_id);

	Q_INVOKABLE void get_file(int server_index, int team_index, int channel_type,
	                          int channel_index, int message_index, int file_index);
	
	Q_INVOKABLE void get_file(FilePtr file);
	
	Q_INVOKABLE void post_file_upload(int server_index, int team_index, int channel_type,
	                                  int channel_index, QString file_path);
	Q_INVOKABLE void post_send_message(QString message, int server_index, int team_index, int channel_type,
	                                   int channel_index, QString root_id = QString());
	Q_INVOKABLE void delete_message(int server_index, int team_index, int channel_type,
	                                     int channel_index, int message_index, QString message_id = QString());
	Q_INVOKABLE void put_message_edit(QString text, int server_index, int team_index, int channel_type,
	                                     int channel_index, int message_index);

	Q_INVOKABLE void post_channel_view(int server_index, int team_index,
	                                   int channel_type, int channel_index);
	/**
	 * @brief post_add_user_to_channel add/join user to channel
	 * @param server_index   as is
	 * @param team_index     as is (really not need)
	 * @param channel_id     as is
	 * @param user_id        if empty - thet mean current user (sever_ptr->m_user_id)
	 */
	Q_INVOKABLE void post_add_user_to_channel(int server_index, int team_index, QString channel_id, 
	                                          QString user_id = QString());
	/**
	 * @brief delete_user_from_channel  delete user from channel (leave channel
	 * if user_id is empty) 
	 * @param server_index  as is
	 * @param channel_id    as is
	 * @param user_id       if empty, than current user 
	 */
	Q_INVOKABLE void delete_user_from_channel(int server_index, QString channel_id, 
	                                          QString user_id = QString());
	
	/**
	 * @brief post_create_channel
	 * @param server_index   as is
	 * @param team_index     as is 
	 * @param is_private     if true create private chanel, else create public channel
	 * @param name           name for new channel
	 * @param header         channels header ( not required )
	 * @param description    channels description/puprose ( not required ) 
	 * @param url_string     if we want set specific url for channel (not released now)
	 */
	Q_INVOKABLE void post_create_channel(int server_index, int team_index, bool is_private, QString name, 
	                                     QString header = QString(), 
	                                     QString purpose = QString(), 
	                                     QString url_name = QString());
	
	Q_INVOKABLE void get_user_image(int server_index, int user_index);
	Q_INVOKABLE void get_user_info(int server_index, QString userId,  int team_index = -1);
	Q_INVOKABLE void get_teams_unread(int server_index);
//	Q_INVOKABLE void get_users(int server_index);

//	Q_INVOKABLE void get_posts(int server_index, int team_index, QString channel_id);
	/**
	 * @brief get_post download post by its id
	 * @param post_id  post id
	 * @param message  message ptr, if message is answer, and post_id is root_id
	 */
	Q_INVOKABLE void get_post(int server_index, QString post_id, MessagePtr message = MessagePtr());
	Q_INVOKABLE void get_posts(int server_index, int team_index, int channel_type, int channel_index);
	Q_INVOKABLE void get_posts_before(int server_index, int team_index, int channel_index, int channel_type);
	Q_INVOKABLE void post_users_status(int server_index);

	Q_INVOKABLE void post_create_reaction(int server_index, const QString &post_id, const QString &emoji_name) const;
	Q_INVOKABLE void delete_reaction(int server_index, const QString &post_id, const QString &emoji_name) const;
	/** get current user id */
	Q_INVOKABLE QString user_id(int server_index) const;

	/**
	 * @brief user_role  return true if role for user is enabled
	 * @param server_index
	 * @param user_index    user index, or -2 for current user
	 * @param role          UserSystemRole
	 * @return  true, if role is enabled for user
	 */
	Q_INVOKABLE bool    user_role(int server_index, int user_index, int role) const; /**< get role for user*/
	Q_INVOKABLE QString getUserName(int server_index, int user_index);
	/** get channel name */
	Q_INVOKABLE QString getChannelName(int server_index, int team_index, int channel_type, int channel_index);
	Q_INVOKABLE QString getChannelId(int server_index, int team_index, int channel_type, int channel_index);
	/** get team data */
	Q_INVOKABLE QString getTeamId(int server_index, int team_index);
	Q_INVOKABLE QString getTeamName(int server_index, int team_index);
	/** functions, called from DBusAdaptor */
	Q_INVOKABLE void notificationActivated(int server_index, int team_index, int channel_type, int channel_index);

	/** Markdown support */
	Q_INVOKABLE QString parseMD(const QString &input) const;

	/** Compute cache size */
	Q_INVOKABLE QString cacheSize() const;
	Q_INVOKABLE void    clearCache();

	/** File manipulations */
	/**
	 * @brief isImageFileInGallery
	 * @param server_index   index of server in servers list
	 * @param file_sc_index  index of file in servers files list
	 * @return  true if file already in gallery
	 */
	Q_INVOKABLE bool isImageFileInGallery(int server_index, int file_sc_index);

	/**
	 * @brief saveImageFileToGallery
	 * @param server_index   index of server in servers list
	 * @param file_sc_index  index of file in servers files list
	 * @return  true if file saved succesfuly
	 */
	Q_INVOKABLE QString saveImageFileToGallery(int server_index, int file_sc_index);

	/** settings  fucntions */
	bool save_settings();
	bool load_settings();

	/** */
	TeamPtr teamAt(int server_index, int team_index);

	ChannelPtr channelAt(int server_index, int team_index,
	                     int channel_type, int channel_index);

	MessagePtr messageAt(int server_index, int team_index,
	                     int channel_type, int channel_index,
	                     int message_index);

	UserPtr  userAt(int server_index, int user_index);

	inline const QVector<ServerPtr> &server() const { return m_server; }

	/** sets pointer to SettingsContainer */
	Q_INVOKABLE void setSettingsContainer(SettingsContainer *settings);

	SettingsContainer *settings();
	/**
	 * @brief messageUnread
	 * get count of message unread for all teams and accounts
	 * @return unread count
	 */
	int messageUnread() const;

Q_SIGNALS:
	void libraryIsInitialized();
	void onApplciationStatusChanged();
	void messageUnreadChanged();

	void serverAdded(ServerPtr server);
	void serverConnected(int server_index);
	void serverStateChanged(int server_index, int state);
	void serverChanged(ServerPtr server,  QVectorInt roles);
	void onConnectionError(int code, QString message, int server_index);
	void channelsList(QList<ChannelPtr> list);
	void channelAdded(ChannelPtr channel);
	void channelRemoved(ChannelPtr channel);
	void updateChannel(ChannelPtr channel,  QVectorInt roles);
	void updateChannelInfo(QString channel_id, int team_index, int self_index );
	void onReplyChannelMembers(ChannelPtr channel, QVector<UserPtr> members);
	void showTeamPublicChannels(QList<ChannelPtr> channels);
//	void updateChannel()
	void messagesAdded(ChannelPtr channel);
	void messagesIsEnd(ChannelPtr channel); // its mean no more messages in channel
	void messagesAddedBefore(ChannelPtr channel, int count);
	void messageAdded(QList<MessagePtr> messages);
	void newMessage(MessagePtr message);
	void messageUpdated(QList<MessagePtr> messages);
	void updateMessage(MessagePtr m,int role);
	void messageBeginDelete(QList<MessagePtr> messages);
	void messageDeleted(QList<MessagePtr> messages);
	void messageEndDelete();
	
	void userAdded(UserPtr user);
	void userUpdated(UserPtr user,  QVectorInt roles);
	void currentUserChanged(UserPtr user);
	void usersUpdated(QVector<UserPtr> users,  QVectorInt roles);
	
	void fileStatusChanged(QString file_id, int status);
	void fileUploaded(int server_index, int file_sc_index);
	// teams changing signals
	void teamAdded(TeamPtr team);
	void teamsExists(const QVector<MattermostQt::TeamPtr> &teams);
	void teamUnread(QString team_id, int msg, int mention);
	void teamChanged(TeamPtr team,  QVectorInt roles);
	void onReplyTeamMembers(TeamPtr team, QVector<UserPtr> members);

	/**
	 * @brief onImageFileSavedToGallery
	 * when we save image to gallery from cache? this signal emmited if it success
	 * @param server_index   server index
	 * @param file_sc_index  file index in server's files list
	 */
	void onImageFileSavedToGallery(int server_index, int file_sc_index);

	/// renew signals
	void attachedFilesChanged(MattermostQt::MessagePtr message, QVector<QString> file_ids,  QVectorInt roles);
//	void fileUploaded(FilePtr file);

	/**
	 * @brief fileDownloadingProgress
	 * @param file_id    id of file
	 * @param progress   from 0.0 to 1.0
	 */
	void fileDownloadingProgress(QString file_id, qreal progress);

	/**
	 * @brief fileUploadProgress
	 * @param data      some string
	 * @param progress  from 0 to 100
	 */
	void fileUploadProgress(QString data, int progress);

	/**
	 * @brief settingsChanged
	 * when settings are changed
	 */
	void settingsChanged();

	/**
	 * @brief requestFinished
	 * shoud call on every reply fucntion
	 * @param request - type of request
	 */
	void requestFinished(ReplyType request);
	
	/**
	 * @brief replyError
	 * @param request
	 * @param error
	 */
	void replyError(ReplyType request, QJsonObject error);
protected:
	/**
	 * @brief prepare_direct_channel
	 * when we have direct channel? we need get user id, login and profile
	 * image before we send to ChannelsModel
	 * @param channel
	 */
	void prepare_direct_channel(int server_index, int channel_index);

	/**
	 * @brief prepare_user_index
	 * @param server_index
	 * @param message
	 */
	void prepare_user_index(int server_index, MessagePtr message, int tean_index = -1);

	/**
	 * @brief find_team_by_id
	 * get team ptr by id string
	 * @param sc       server ptr
	 * @param team_id  team id
	 * @return  empty or valid team ptr
	 */
	TeamPtr find_team_by_id(ServerPtr sc, QString team_id) const;

	inline ServerPtr get_server(int server_index) const;

	void get_teams_unread(ServerPtr server);

	void websocket_connect(ServerPtr server);
	// repliest
	void reply_login(QNetworkReply *reply);
	void reply_get_teams(QNetworkReply *reply);

	void reply_get_team(QNetworkReply *reply);
	void reply_get_teams_unread(QNetworkReply *reply);
	void reply_get_team_icon(QNetworkReply *reply);
	void reply_get_team_members(QNetworkReply *reply);
	void reply_get_team_statistics(QNetworkReply *reply);
	void reply_get_post(QNetworkReply *reply);
	void reply_get_posts(QNetworkReply *reply);
	void reply_get_posts_before(QNetworkReply *reply);
	void reply_get_public_channels(QNetworkReply *reply);
	void reply_get_team_public_channels(QNetworkReply *reply);
	void reply_get_channel(QNetworkReply *reply);
	void reply_get_channel_unread(QNetworkReply *reply);
	void reply_get_channel_members(QNetworkReply *reply);
	void reply_get_channel_statistics(QNetworkReply *reply);
	void reply_post_channel_view(QNetworkReply *reply);
	void reply_post_add_user_to_channel(QNetworkReply *reply);
	void reply_delete_user_from_channel(QNetworkReply *reply);
	void reply_post_create_channel(QNetworkReply *reply);
	void reply_get_user_info(QNetworkReply *reply);
	void error_get_user_info(QNetworkReply *reply);
	void reply_post_users_status(QNetworkReply *reply);
	void reply_error(QNetworkReply *reply);
	void reply_get_file_thumbnail(QNetworkReply *reply);
	void reply_get_file_preview(QNetworkReply *reply);
	void reply_get_file_info(QNetworkReply *reply);
	void reply_get_file(QNetworkReply *reply);
	void reply_post_file_upload(QNetworkReply *reply);
	void reply_get_user_image(QNetworkReply *reply);
	void reply_post_send_message(QNetworkReply *reply);
	void reply_delete_message(QNetworkReply *reply);
	void reply_put_message_edit(QNetworkReply *reply);
	void reply_post_create_reaction(QNetworkReply *reply);
	void reply_delete_reaction(QNetworkReply *reply);
	// failed replies
	void failed_get_file_info(QNetworkReply *reply);
	// events
	void event_posted(ServerPtr sc, QJsonObject object);
	void event_post_edited(ServerPtr sc, QJsonObject object);
	void event_post_deleted(ServerPtr sc, QJsonObject object);
	void event_status_change(ServerPtr sc, QJsonObject object);
	void event_typing(ServerPtr sc, QJsonObject object);
	void event_channel_viewed(ServerPtr sc, QJsonObject object);
	void event_reaction_added(ServerPtr sc, QJsonObject object);
	void event_reaction_removed(ServerPtr sc, QJsonObject object);
	void event_channel_created(ServerPtr sc, QJsonObject object);
	void event_user_added(ServerPtr sc, QJsonObject object);
	void event_user_removed(ServerPtr sc, QJsonObject object);

	// helper functions
	inline UserStatus str2status(const QString &s) const;
	inline UserPtr    id2user(ServerPtr sc, const QString &id) const;
	inline TeamPtr    id2team(ServerPtr sc, const QString &id) const;
	inline ChannelPtr id2channel(ServerPtr sc, const QString &id) const;
	inline MessagePtr id2message(ChannelPtr channel, const QString &id) const;
//	void  message_format(MessagePtr message);
	/**
	 * @brief generateCachePath
	 * @param isOk if cache generated wrong, it set to false ( for fixing paths
	 * @return cache path
	 */
	const QString generateCachePath(bool *isOk = nullptr) const;
//	const QString generateDataPath() const;
protected Q_SLOTS:
	void replyFinished(QNetworkReply *reply);
	void slotNetworkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility accessible);
	void replySSLErrors(QNetworkReply *reply, QList<QSslError> errors);
	void replyDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
	void replyUploadProgress(qint64 bytesReceived, qint64 bytesTotal);

	void onWebSocketConnected();
	void onWebSocketSslError(QList<QSslError> errors);
	void onWebSocketError(QAbstractSocket::SocketError error);
	void onWebSocketStateChanged(QAbstractSocket::SocketState state);
//	void onWebSocketTextFrameReceived(const QString &frame, bool isLastFrame);
	void onWebSocketTextMessageReceived(const QString &message);
	void onWebSocketPong(quint64 elapsedTime,QByteArray payload);

	/** slot for QTimer */
	void slot_get_teams_unread();
	void slot_recconect_servers();
	void slot_user_status();
	void slot_ping_timeout();
	/** */
	void slot_settingsChanged();
	void slot_channelAdded(ChannelPtr channel);
protected:
	QVector<ServerPtr>    m_server;
	QSharedPointer<QNetworkAccessManager>  m_networkManager;
//	QSharedPointer<SettingsContainer>    m_settings;
	SettingsContainer    *m_settings;
	MarkdownParser       *m_mdParser; /**< simple Markdown parser insterface */

	QString m_config_path;
	QString m_data_path;
	QString m_cache_path;
	QString m_pictures_path;
	QString m_documents_path;
	QString m_download_path;
//	QTimer  m_settings_timer;

	int    m_update_server_timeout;
	QTimer m_reconnect_server;
	// TODO make get user status when server is connected, and users is grabbed
	int    m_user_status_timeout;
	QTimer m_user_status_timer;
	// ping server timer
	int    m_ping_server_timeout;
	/** channels, need update before put to model */
	//	QList<ChannelContainer>   m_stackedChannels;
	ApplicationStatus m_appStatus = ApplicationStatus::AppInactive;
};

// Second, define the singleton type provider function (callback).
static QObject *MattermostQt_singletontype_provider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
	Q_UNUSED(engine)
	Q_UNUSED(scriptEngine)
	return MattermostQt::getInstance();
}

#endif // MATTERMOSTQT_H
