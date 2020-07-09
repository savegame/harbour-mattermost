//#include "MattermostQt.h"
#include <algorithm>

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QStandardPaths>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QUrlQuery>
#include <QDebug>
#include <QHttpMultiPart>
#include <QHttpPart>
#include <QMimeDatabase>
#include <QMimeType>
#include <QCoreApplication>
//#include "libs/qtwebsockets/include/QtWebSockets/qwebsocket.h"
//#include <QWebSocket>

#include "MessagesModel.h"
#include "ChannelsModel.h"
#include "TeamsModel.h"
#include "SettingsContainer.h"
#include "AttachedFilesModel.h"
#include "DiscountMDParser.h"
#include "AccountsModel.h"
#include "cpphash.h"

// all properties names
#define P_REPLY_TYPE         "reply_type"
#define P_API                "api"
#define P_SERVER_URL         "server_url"
#define P_SERVER_NAME        "server_name"
#define P_SERVER_INDEX       "server_index"
#define P_USER_INDEX         "user_index"
#define P_USER_ID            "user_id"
#define P_FILE_SC_INDEX      "file_sc_index"
#define P_TEAM_INDEX         "team_index"
#define P_MESSAGE_INDEX      "message_index"
#define P_FILE_INDEX         "file_index"
#define P_FILE_ID            "file_id"
#define P_FILE_PTR           "file_ptr"
#define P_FILE_PATH          "file_path"
#define P_MESSAGE_PTR        "message_ptr"
#define P_CHANNEL_PTR        "channel_ptr"
#define P_TEAM_ID            "team_id"
#define P_CHANNEL_INDEX      "channel_index"
#define P_CHANNEL_ID         "channel_id"
#define P_CHANNEL_TYPE       "channel_type"
#define P_TRUST_CERTIFICATE  "trust_certificate"
#define P_DIRECT_CHANNEL     "direct_channel"
#define P_CA_CERT_PATH       "ca_cert_path"
#define P_CERT_PATH          "cert_path"
#define P_NEED_SAVE_SETTINGS "save_settings"
// config file name
#define F_CONFIG_FILE        "config.json"
#define F_SETTINGS_FILE      "settings.json"
// some helpers defines
#define cmp(s,t) s.compare(#t) == 0
#define scmp(s1,s2) s1.compare(s2) == 0
#define _compare(string) if( cmp(event,string) )
// additioanl get_user_info flags
#define GET_USER_INFO_direct_channel -1 // reques user info for direct channel
#define GET_USER_INFO_current_user   -2 // request user info for current loggined user

//#ifdef _RELEASE
#define bearer_string(token) QString("Bearer %0").arg(token)

#define request_urlencoded(requset) \
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-urlencoded")

#define request_set_headers(requset, server) \
	request_urlencoded(request); \
	request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("Sailfish Mattermost v%0").arg(MATTERMOSTQT_VERSION) ); \
	if( !server->m_cookie.isEmpty() ) \
	request.setHeader(QNetworkRequest::CookieHeader, server->m_cookie); \
	request.setRawHeader("Authorization", server->m_bearer.toUtf8())

#define request_json(requset) \
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json")


Q_DECLARE_METATYPE(MattermostQt::FilePtr)
Q_DECLARE_METATYPE(MattermostQt::ChannelPtr)
Q_DECLARE_METATYPE(MattermostQt::MessagePtr)

#define ASSERT_REPLY(function_name) \
	if(!reply) { \
	    qCritical() << QStringLiteral("Function %0 call with NULL QNetworkReply pointer.").arg( #function_name ); \
	    return; \
	}

#define BIND_REPLY_FUNCTION(name) m_reply_func[ReplyType:: CONCAT2(rt_,name) ] = &MattermostQt:: CONCAT2(reply_, name)
#define BIND_EVENT_FUNCTION(name) m_event_func[EventType:: CONCAT2(et_,name) ] = &MattermostQt:: CONCAT2(event_, name)

MattermostQt *MattermostQt::getInstance()
{
	static MattermostQt *mattermost = new MattermostQt();
	static bool first_call = true;
	if( first_call ) {
		first_call = false;
		qInfo() << QStringLiteral("MattermostQt::getInstance() first call: MattermostQt static object alloicated.");
	}
	return mattermost;
}

void MattermostQt::bind_reply_functions()
{
	memset(m_reply_func, NULL, sizeof(void*)*ReplyTypeCount );
	//func_pointer = (reply_func)&MattermostQt::reply_login;
	BIND_REPLY_FUNCTION(login);
	BIND_REPLY_FUNCTION(get_teams);
	BIND_REPLY_FUNCTION(get_team_icon);
	BIND_REPLY_FUNCTION(get_public_channels);
	BIND_REPLY_FUNCTION(get_channel);
	BIND_REPLY_FUNCTION(post_channel_view);
	BIND_REPLY_FUNCTION(get_user_info); //-
	BIND_REPLY_FUNCTION(post_users_status);
	BIND_REPLY_FUNCTION(get_user_image);
	BIND_REPLY_FUNCTION(get_team);
	BIND_REPLY_FUNCTION(get_teams_unread);
	BIND_REPLY_FUNCTION(get_post);
	BIND_REPLY_FUNCTION(get_posts);
	BIND_REPLY_FUNCTION(get_posts_before);
	BIND_REPLY_FUNCTION(get_file_thumbnail);
	BIND_REPLY_FUNCTION(get_file_preview);
	BIND_REPLY_FUNCTION(get_file_info);
	BIND_REPLY_FUNCTION(get_file);
	BIND_REPLY_FUNCTION(post_file_upload);
	BIND_REPLY_FUNCTION(post_send_message);
	BIND_REPLY_FUNCTION(delete_message);
	BIND_REPLY_FUNCTION(put_message_edit);
	BIND_REPLY_FUNCTION(get_channel_unread);
	BIND_REPLY_FUNCTION(post_create_reaction);
	BIND_REPLY_FUNCTION(delete_reaction);
}

void MattermostQt::bind_event_functions()
{
//	staticMetaObject.indexOfEnumerator("ReplyType");
//	for(int et = 0; et < EventTypeCount; et++ )
//	{
//		const QMetaObject *mo = qt_getEnumMetaObject( (EventType)et );
//		const char *name = qt_getEnumName( (EventType)et );
//		int enumIdx = mo->indexOfEnumerator(name);
//		qDebug() << QStringLiteral("ET: %0 with Idx( %1 ) and i( %2 );").arg(name).arg(enumIdx).arg(et);
//		QVariant e = QVariant::fromValue<EventType>((EventType)et);
//qDebug() << QStringLiteral("EventType: ") << (EventType)et << qt_getEnumName( (EventType)et );;
//		qDebug() << QStringLiteral("From QVariant: %0").arg(e.toString());
//		QVariant e2 = QVariant( e.toString() );
//		qDebug() << QStringLiteral("From string to Variant int: [%0] %1").arg((int)e2.value<EventType>()).arg(e2.toString());
//	}
//	int enumIdx = mo->indexOfEnumerator(qt_getEnumName(enumValue));
//	return dbg << mo->enumerator(enumIdx).valueToKey(enumValue);

	memset( m_event_func, NULL, sizeof(void*)*EventTypeCount );

	//	BIND_EVENT_FUNCTION(name);
	BIND_EVENT_FUNCTION(posted);
	BIND_EVENT_FUNCTION(post_edited);
	BIND_EVENT_FUNCTION(post_deleted);
	BIND_EVENT_FUNCTION(status_change);
	BIND_EVENT_FUNCTION(typing);
	BIND_EVENT_FUNCTION(channel_viewed);
	BIND_EVENT_FUNCTION(reaction_added);
	BIND_EVENT_FUNCTION(reaction_removed);
}

MattermostQt::MattermostQt(QObject *parent )
    : QObject(parent)
    , m_mdParser(nullptr)
    , m_settings(nullptr)
{
	bind_reply_functions();
	bind_event_functions();
	m_networkManager.reset(new QNetworkAccessManager());

	m_update_server_timeout = 5000; // in millisecs
	m_reconnect_server.setInterval(m_update_server_timeout);
	qDebug() << QStringLiteral("Recconect timer set interval ").arg(m_update_server_timeout);
	m_user_status_timeout = 30000;  // half minute
	m_user_status_timer.setInterval(m_user_status_timeout);
	m_user_status_timer.setSingleShot(false);

	m_ping_server_timeout = 15000; //15 seconds
	//	m_ping_server_timer.setInterval(m_ping_server_timeout);
	//	m_ping_server_timer.setSingleShot(false);

	connect( &m_reconnect_server, SIGNAL(timeout()), SLOT(slot_recconect_servers()) );
	connect( &m_user_status_timer, SIGNAL(timeout()), SLOT(slot_user_status()) );

	connect(m_networkManager.data(), SIGNAL(finished(QNetworkReply*)),
	        this, SLOT(replyFinished(QNetworkReply*)));
	connect(m_networkManager.data(), SIGNAL(networkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility)),
	        this, SLOT(slotNetworkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility)));
	connect(m_networkManager.data(),SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)),
	        this, SLOT(replySSLErrors(QNetworkReply*,QList<QSslError>)));

	m_config_path = QDir(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation))
	        .filePath(QCoreApplication::applicationName());

	m_data_path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
	if( m_data_path.lastIndexOf( QCoreApplication::organizationName()) != -1 )
	{	// this wrong path, try fix it
		// its /home/nemo/.local/share/sashikknox/harbour-mattermost

		QString wrong_path = m_data_path + QStringLiteral("/");
		QDir dataDir(wrong_path);
		m_data_path.remove( m_data_path.lastIndexOf( QCoreApplication::organizationName()) , QCoreApplication::organizationName().append("/").length() );

		if( dataDir.exists() )
		{ // try it move to right location
			if( dataDir.rename(wrong_path, m_data_path) ) {
				qInfo() << QStringLiteral("Data dir moved from '%0' to '%1'").arg(wrong_path).arg(m_data_path);
				wrong_path.remove(QCoreApplication::applicationName().append("/"));
				QDir removeDir( wrong_path );
				if( removeDir.removeRecursively() )
					qWarning() << QStringLiteral("Old data dir removed %0").arg(wrong_path);
			}
			else
				qWarning() << QStringLiteral("Cant move dir from '%0' to '%1'").arg(wrong_path).arg(m_data_path);
		}
	}

	m_cache_path = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
	if( m_cache_path.lastIndexOf( QStringLiteral(".cache/").arg(QCoreApplication::organizationName()) ) != -1 )
	{	// this wrong path, try fix it
// its /home/nemo/.local/share/sashikknox/harbour-mattermost

		QString wrong_path = m_cache_path + QStringLiteral("/");
		QDir cacheDir(wrong_path);
		m_cache_path.remove( m_cache_path.lastIndexOf( QCoreApplication::organizationName()) , QCoreApplication::organizationName().append("/").length() );

		if( cacheDir.exists() )
		{ // try it move to right location
			if( cacheDir.rename(wrong_path, m_cache_path) ) {
				qInfo() << QStringLiteral("Cache dir moved from '%0' to '%1'").arg(wrong_path).arg(m_cache_path);
//				dataDir.remove( wrong_path.remove(QCoreApplication::applicationName().append("/")) );
				wrong_path.remove(QCoreApplication::applicationName().append("/"));
				QDir removeDir( wrong_path );
				if( removeDir.removeRecursively() )
					qWarning() << QStringLiteral("Old cache dir removed %0").arg(wrong_path);
			}
			else
				qWarning() << QStringLiteral("Cant cache move dir from '%0' to '%1'").arg(wrong_path).arg(m_cache_path);
		}
	}

	if( m_cache_path.isEmpty() ) {
		qCritical() << QStringLiteral("Cant get CacheLocation");
		//		m_cache_path = "/home/nemo/.cache";
	}

	qDebug() << QStringLiteral("Global data path is: %0").arg(m_data_path);
	qDebug() << QStringLiteral("Global cache path is: %0").arg(m_cache_path);

	m_documents_path = QDir(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation))
	        .filePath(QCoreApplication::applicationName());
	m_pictures_path = QDir(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation))
	        .filePath(QCoreApplication::applicationName());
	m_download_path = QDir(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation))
	        .filePath(QCoreApplication::applicationName());

	m_settings = SettingsContainer::getInstance();
	//	m_settings.reset(new SettingsContainer(this));
	connect(m_settings, &SettingsContainer::settingsChanged, this, &MattermostQt::slot_settingsChanged);
	connect(this, &MattermostQt::channelAdded, this, &MattermostQt::slot_channelAdded);

	m_mdParser = new DiscountMDParser();

	load_settings();

	emit libraryIsInitialized();
}

MattermostQt::~MattermostQt()
{
	m_server.clear();

	delete m_mdParser;
}

MattermostQt::ApplicationStatus MattermostQt::getApplicationStatus() const
{
	return m_appStatus;
}

void MattermostQt::setApplicationStatus(MattermostQt::ApplicationStatus status)
{
	if( m_appStatus == status )
		return;
	m_appStatus = status;
	qDebug() << QStringLiteral("Appcliaction status changed: %0").arg( QVariant::fromValue<ApplicationStatus>( status ).toString() );
	emit onApplciationStatusChanged();
}

QString MattermostQt::getVersion() const
{
	return MATTERMOSTQT_VERSION;
}

QString MattermostQt::emojiPath() const
{
	return QStringLiteral(EMOJI_PATH);
}

int MattermostQt::get_server_state(int server_index)
{
	if(server_index < 0 || server_index >= m_server.size() )
		return -1;
	return m_server[server_index]->m_state;
}

void MattermostQt::force_server_recconect()
{
	slot_recconect_servers();
}

int MattermostQt::get_server_count() const
{
	return m_server.size();
}

QString MattermostQt::get_server_name(int server_index) const
{
	if( server_index < 0 || server_index >= m_server.size() )
		return QString();
	return m_server[server_index]->m_display_name;
}

QString MattermostQt::get_server_url(int server_index) const
{
	if( server_index < 0 || server_index >= m_server.size() )
		return QString();
	return m_server[server_index]->m_url;
}

bool MattermostQt::get_server_trust_certificate(int server_index) const
{
	if( server_index < 0 || server_index >= m_server.size() )
		return false;
	return m_server[server_index]->m_trust_cert;
}

QString MattermostQt::get_server_cert_path(int server_index) const
{
	if( server_index < 0 || server_index >= m_server.size() )
		return QString();
	//	if (m_server[server_index]->m_cert_path.isEmpty())
	//	{
	//		m_server[server_index]->m_cert_path = m_server[server_index]->m_data_path + QDir::separator() + QLatin1String("server.crt");
	//	}
	return m_server[server_index]->m_cert_path;
}

QString MattermostQt::get_server_ca_cert_path(int server_index) const
{
	if( server_index < 0 || server_index >= m_server.size() )
		return QString();
	return m_server[server_index]->m_ca_cert_path;
}

void MattermostQt::set_server_enabled(int server_index, const bool enabled)
{
	if( server_index < 0 || server_index >= m_server.size() )
		return;
	ServerPtr server = m_server[server_index];
	if( enabled == server->m_enabled )
		return;
	server->m_enabled = enabled;

	emit serverChanged(server,  QVectorInt() << AccountsModel::RoleIsEnabled );
	if(server->m_enabled )
	{
		server->m_ping_timer.stop();
		qDebug() << QStringLiteral("Server[%0] (%1)  stop ping timer.").arg(server->m_self_index).arg(server->m_display_name);
		server->m_state = ServerUnconnected;
		emit serverStateChanged(server_index, ServerUnconnected);
	}
	else {
		//		get_login(server);
		slot_recconect_servers();
		m_reconnect_server.start();
		qDebug() << QStringLiteral("Server[%0] (%1) recconect timer started.").arg(server->m_self_index).arg(server->m_display_name);
	}
}

void MattermostQt::post_login(QString server, QString login, QString password,
                              int api,QString display_name,
                              bool trustCertificate, QString ca_cert_path, QString cert_path)
{
	if(api <= 3)
		api = 4;

	// {"login_id":"someone@nowhere.com","password":"thisisabadpassword"}

	QString urlString = QLatin1String("/api/v")
	        + QString::number(api)
	        + QLatin1String("/users/login");
	//	QRegExp hasPort(":[0-9]+");
	//	if( server.indexOf("https://") >= 0 && )
	//		server.append(":443")

	QUrl url(server);
	url.setPath(url.path() + urlString);
	QNetworkRequest request;
	QJsonDocument json;
	QJsonObject data;

	data["login_id"] = login;
	data["password"] = password;

	json.setObject(data);

	request.setUrl(url);
	request.setHeader(QNetworkRequest::ServerHeader, "application/json; charset=utf-8");
	request.setHeader(QNetworkRequest::ContentLengthHeader, QByteArray::number( json.toJson().size() ));
	request.setHeader(QNetworkRequest::UserAgentHeader, QString("MattermosQt v%0").arg(MATTERMOSTQT_VERSION) );
	request_urlencoded(request);
	request.setRawHeader("X-Custom-User-Agent", QString("MattermosQt v%0").arg(MATTERMOSTQT_VERSION).toUtf8());

	QNetworkReply *reply = m_networkManager->post(request, json.toJson() );
	reply->setProperty(P_REPLY_TYPE, QVariant(ReplyType::rt_login) );
	reply->setProperty(P_API, QVariant(api) );
	reply->setProperty(P_SERVER_URL, server );
	reply->setProperty(P_SERVER_NAME, display_name );
	reply->setProperty(P_CA_CERT_PATH, ca_cert_path );
	reply->setProperty(P_CERT_PATH, cert_path );
	reply->setProperty(P_TRUST_CERTIFICATE, trustCertificate );

	//	// Load previosly saved certificate
	if( trustCertificate )
	{
		QList<QSslError> errors;

		if(ca_cert_path.isEmpty())
			qInfo() << "Path to CA certificate file is empty";
		else
		{
			QFile ca_cert_file(ca_cert_path);
			if( ca_cert_file.open(QIODevice::ReadOnly) )
			{
				QSslCertificate ca_cert(&ca_cert_file, QSsl::Pem);
				errors << QSslError(QSslError::CertificateUntrusted, ca_cert);
				errors << QSslError(QSslError::SelfSignedCertificateInChain, ca_cert);
				errors << QSslError(QSslError::SelfSignedCertificate, ca_cert);
				ca_cert_file.close();
			}
			else
				qCritical() << "Cant open ca crt file " << ca_cert_path;
		}

		if(cert_path.isEmpty())
			qInfo() << "Path to certificate file is empty";
		else
		{
			QFile cert_file(cert_path);
			if( cert_file.open(QIODevice::ReadOnly) )
			{
				QSslCertificate cert(&cert_file, QSsl::Pem);
				errors << QSslError(QSslError::CertificateUntrusted, cert);
				errors << QSslError(QSslError::SelfSignedCertificateInChain, cert);
				errors << QSslError(QSslError::SelfSignedCertificate, cert);
				cert_file.close();
			}
			else
				qCritical() << "Cant open crt file " << cert_path;
		}

		errors << QSslError(QSslError::CertificateUntrusted);
		errors << QSslError(QSslError::SelfSignedCertificateInChain);
		errors << QSslError(QSslError::SelfSignedCertificate);
		reply->ignoreSslErrors(errors);
	}
}

void MattermostQt::post_login_by_token(QString url, QString token, int api, QString display_name, bool trustCertificate, QString ca_cert_path, QString cert_path)
{
	//	QString url = QString("%0")
	ServerPtr server( new ServerContainer(url,token,api) );
	server->m_trust_cert = trustCertificate;
	server->m_display_name = display_name;
	server->m_self_index = m_server.size();
	server->m_ca_cert_path = ca_cert_path;
	server->m_cert_path = cert_path;
	//	server->m_user_id = user_id;
	server->m_ping_timer.setInterval( m_ping_server_timeout );
	qDebug() << QStringLiteral("Server[%0] (%1) set ping timer interval to %2.").arg(server->m_self_index).arg(server->m_display_name).arg(m_ping_server_timeout);
	m_server.append(server);
	get_login(server);
	emit serverAdded(server);
}

void MattermostQt::get_login(MattermostQt::ServerPtr sc)
{
	if( sc->m_state == ServerLogin )
		return;
	sc->m_state = ServerLogin;
	// fix api minimum version
	if(sc->m_api <= 3)
		sc->m_api = 4;

	QString urlString = QLatin1String("/api/v")
	        + QString::number(sc->m_api)
	        + QLatin1String("/users/me");

	QUrl url(sc->m_url);
	url.setPath(url.path() + urlString);
	QNetworkRequest request;

	request.setUrl(url);
	request_set_headers(request,sc);
	request_urlencoded(request);

	QNetworkReply *reply = m_networkManager->get(request);
	reply->setProperty(P_REPLY_TYPE, QVariant(ReplyType::rt_login) );
	reply->setProperty(P_SERVER_INDEX, sc->m_self_index);
	reply->setProperty(P_TRUST_CERTIFICATE, sc->m_trust_cert);

	// Load previosly saved certificate
	if( sc->m_trust_cert )
	{
		QFile ca_cert_file(sc->m_ca_cert_path);
		QFile cert_file(sc->m_cert_path);
		QList<QSslError> errors;
		if( ca_cert_file.open(QIODevice::ReadOnly) )
		{
			QSslCertificate ca_cert(&ca_cert_file, QSsl::Pem);
			errors << QSslError(QSslError::CertificateUntrusted, ca_cert);
			errors << QSslError(QSslError::SelfSignedCertificateInChain, ca_cert);
			errors << QSslError(QSslError::SelfSignedCertificate, ca_cert);
			ca_cert_file.close();
		}
		if( cert_file.open(QIODevice::ReadOnly) )
		{
			QSslCertificate cert(&cert_file, QSsl::Pem);
			errors << QSslError(QSslError::CertificateUntrusted, cert);
			errors << QSslError(QSslError::SelfSignedCertificateInChain, cert);
			errors << QSslError(QSslError::SelfSignedCertificate, cert);
			cert_file.close();
		}
		errors << QSslError(QSslError::CertificateUntrusted);
		errors << QSslError(QSslError::SelfSignedCertificateInChain);
		errors << QSslError(QSslError::SelfSignedCertificate);
		reply->ignoreSslErrors(errors);
	}
}

void MattermostQt::get_teams(int server_index)
{
	/* TODO first, check teams from filesystem,
	 then send request about lst change date    */
	if( server_index < 0 || server_index >= m_server.size() )
		return;
	ServerPtr sc = m_server[server_index];

	if( !sc->m_teams.isEmpty() )
	{
		emit teamsExists(sc->m_teams);
		return;
	}

	QString urlString = QLatin1String("/api/v")
	        + QString::number(sc->m_api)
	        //	        + QLatin1String("/users/")
	        //	        + sc->m_user_id
	        //	        + QLatin1String("/teams");
	        + QLatin1String("/users/me/teams");

	QUrl url(sc->m_url);
	url.setPath(url.path() + urlString);
	QNetworkRequest request;

	request.setUrl(url);
	request_set_headers(request,sc);
	request_urlencoded(request);

	if(sc->m_trust_cert)
		request.setSslConfiguration(sc->m_cert);

	//	foreach( QByteArray n, request.rawHeaderList() )
	//	{
	//		qDebug() << n << request.rawHeader(n);
	//	}
	QNetworkReply *reply = m_networkManager->get(request);
	reply->setProperty(P_REPLY_TYPE, QVariant(ReplyType::rt_get_teams) );
	reply->setProperty(P_SERVER_INDEX, QVariant(server_index) );
}

void MattermostQt::get_team_icon(int server_index, int team_index)
{
	if( server_index < 0 || server_index >= m_server.size() )
		return;
	ServerPtr sc = m_server[server_index];

	if( team_index < 0 || team_index >= sc->m_teams.size() )
	{
		return;
	}
	TeamPtr team = sc->m_teams[team_index];
	qDebug() << QStringLiteral("Request: Get team [%0.%1] id(%2) icon").arg(server_index).arg(team_index).arg(team->m_id);

	//	if(!team->m_image_path.isEmpty())
	//	{
	if( QFile::exists(sc->m_cache_path + QStringLiteral("/teams/%0/image.png").arg(team->m_id) ) )
	{
		team->m_image_path = sc->m_cache_path + QStringLiteral("/teams/%0/image.png").arg(team->m_id);
		QFileInfo fileInfo(team->m_image_path);
		QDateTime modified = fileInfo.lastModified();
		//			fileInfo.
		qlonglong team_modified = modified.toMSecsSinceEpoch();
		// check file date? and teams last update date
		if( team->m_update_at <= modified.toMSecsSinceEpoch() )
		{
			QVectorInt roles;
			roles << TeamsModel::TeamIcon;
			emit teamChanged(team, roles);
			return;
		}
	}
	//	}

	QString urlString = QLatin1String("/api/v")
	        + QString::number(sc->m_api)
	        + QLatin1String("/teams/")
	        + team->m_id
	        + QLatin1String("/image");

	QUrl url(sc->m_url);
	url.setPath(url.path() + urlString);
	QNetworkRequest request;

	request.setUrl(url);
	request_set_headers(request,sc);
	request_urlencoded(request);

	if(sc->m_trust_cert)
		request.setSslConfiguration(sc->m_cert);

	QNetworkReply *reply = m_networkManager->get(request);
	reply->setProperty(P_REPLY_TYPE, QVariant(ReplyType::rt_get_team_icon) );
	reply->setProperty(P_SERVER_INDEX, QVariant(server_index) );
	reply->setProperty(P_TEAM_INDEX, QVariant(team_index) );
}

void MattermostQt::get_public_channels(int server_index, QString team_id)
{
	if( team_id.isNull() || team_id.isEmpty() || server_index < 0 )
	{
		qWarning() << "Wrong team id";
		return;
	}

	if( server_index < 0 || server_index >= m_server.size() )
		return;
	ServerPtr sc = m_server[server_index];

	// first check if team allready got channels
	int team_index = sc->get_team_index(team_id);
	if(team_index == -1)
	{
		qWarning() << "Team with id " << team_id << " not found";
		return;
	}

	TeamPtr tc = sc->m_teams[team_index];
	if( tc->m_private_channels.size() + tc->m_public_channels.size() > 0 )
	{
		QList<ChannelPtr> channels;

		for(int i = 0; i < tc->m_public_channels.size(); i++ )
			channels.append(tc->m_public_channels[i]);

		for(int i = 0; i < tc->m_private_channels.size(); i++ )
			channels.append(tc->m_private_channels[i]);

		for(int i = 0; i < sc->m_direct_channels.size(); i++ )
			channels.append(sc->m_direct_channels[i]);

		emit channelsList( channels );
		// after that send request for team info

		//		get_team(server_index,team_index);
		return;
	}

	QString urlString = QLatin1String("/api/v")
	        + QString::number(sc->m_api)
	        + QLatin1String("/users/me/teams/")
	        + team_id
	        + QLatin1String("/channels");

	QUrl url(sc->m_url);
	url.setPath(url.path() + urlString);
	QNetworkRequest request;

	request.setUrl(url);
	request_set_headers(request,sc);
	request_urlencoded(request);

	QNetworkReply *reply = m_networkManager->get(request);
	reply->setProperty(P_REPLY_TYPE, QVariant(ReplyType::rt_get_public_channels) );
	reply->setProperty(P_SERVER_INDEX, QVariant(server_index) );
	reply->setProperty(P_TEAM_INDEX, QVariant(team_index) );
	reply->setProperty(P_TEAM_ID, QVariant(team_id) );
}

void MattermostQt::get_channel(int server_index, QString channel_id)
{
	ServerPtr sc = get_server(server_index);

	if(!sc) {
		qWarning() << "Wrong server index";
		return;
	}

	QString urlString = QLatin1String("/api/v")
	        + QString::number(sc->m_api)
	        + QLatin1String("/channels/")
	        + channel_id;

	QUrl url(sc->m_url);
	url.setPath(url.path() + urlString);
	QNetworkRequest request;

	request.setUrl(url);
	request_set_headers(request,sc);
	request_urlencoded(request);

	QNetworkReply *reply = m_networkManager->get(request);
	reply->setProperty(P_REPLY_TYPE, QVariant(ReplyType::rt_get_channel) );
	reply->setProperty(P_SERVER_INDEX, QVariant(server_index) );
	reply->setProperty(P_CHANNEL_ID, QVariant(channel_id) );
}

void MattermostQt::get_channel(int server_index, int team_index, int channel_type, int channel_index)
{
	QString channel_id = getChannelId(server_index,team_index,channel_type,channel_index);
	if( channel_id.isEmpty() )
		return;
	get_channel(server_index,channel_id);
}

void MattermostQt::get_team(int server_index, int team_index)
{
	if( server_index < 0 || server_index >= m_server.size()
	        || team_index < 0 || team_index >= m_server[server_index]->m_teams.size() )
	{
		qWarning() << "Wrond indexes";
		return;
	}
	ServerPtr sc = m_server[server_index];
	QString team_id = sc->m_teams[team_index]->m_id;

	QString urlString = QLatin1String("/api/v")
	        + QString::number(sc->m_api)
	        + QLatin1String("/teams/")
	        + team_id;

	QUrl url(sc->m_url);
	url.setPath(url.path() + urlString);
	QNetworkRequest request;

	request.setUrl(url);
	request_set_headers(request,sc);
	request_urlencoded(request);

	QNetworkReply *reply = m_networkManager->get(request);
	reply->setProperty(P_REPLY_TYPE, QVariant(ReplyType::rt_get_team) );
	reply->setProperty(P_SERVER_INDEX, QVariant(server_index) );
	reply->setProperty(P_TEAM_INDEX, QVariant(team_index) );
}

void MattermostQt::get_file_thumbnail(int server_index, int file_sc_index)
{
	if( server_index < 0 || server_index >= m_server.size() )
		return;
	// we think all indexes is right
	ServerPtr sc = m_server[server_index];
	if( file_sc_index < 0 || file_sc_index >= sc->m_file.size() )
		return;
	FilePtr file = sc->m_file[file_sc_index];
	QString file_id = file->m_id;
	//files/{file_id}/thumbnail
	QString urlString = QLatin1String("/api/v")
	        + QString::number(sc->m_api)
	        + QLatin1String("/files/")
	        + file_id
	        + QLatin1String("/thumbnail");

	QUrl url(sc->m_url);
	url.setPath(url.path() + urlString);
	QNetworkRequest request;

	request.setUrl(url);
	request_set_headers(request,sc);
	request_urlencoded(request);

	QNetworkReply *reply = m_networkManager->get(request);
	reply->setProperty(P_TRUST_CERTIFICATE, QVariant(sc->m_trust_cert) );
	reply->setProperty(P_REPLY_TYPE, QVariant(ReplyType::rt_get_file_thumbnail) );
	reply->setProperty(P_SERVER_INDEX, QVariant(server_index) );
	reply->setProperty(P_FILE_SC_INDEX, QVariant(file_sc_index) );
}

void MattermostQt::get_file_preview(int server_index, int file_sc_index)
{
	if( server_index < 0 || server_index >= m_server.size() )
		return;
	// we think all indexes is right
	ServerPtr sc = m_server[server_index];

	if( file_sc_index < 0 || file_sc_index >= sc->m_file.size() )
		return;

	FilePtr file = sc->m_file[file_sc_index];
	QString file_id = file->m_id;
	//files/{file_id}/preview
	QString urlString = QLatin1String("/api/v")
	        + QString::number(sc->m_api)
	        + QLatin1String("/files/")
	        + file_id
	        + QLatin1String("/preview");

	QUrl url(sc->m_url);
	url.setPath(url.path() + urlString);
	QNetworkRequest request;

	request.setUrl(url);
	request_set_headers(request,sc);
	request_urlencoded(request);

	QNetworkReply *reply = m_networkManager->get(request);
	reply->setProperty(P_TRUST_CERTIFICATE, QVariant(sc->m_trust_cert) );
	reply->setProperty(P_REPLY_TYPE, QVariant(ReplyType::rt_get_file_preview) );
	reply->setProperty(P_SERVER_INDEX, QVariant(server_index) );
	reply->setProperty(P_FILE_SC_INDEX, QVariant(file_sc_index) );
}

void MattermostQt::get_file_info(int server_index, int team_index, int channel_type,
                                 int channel_index, int message_index, QString file_id)
{
	// we think all indexes is right
	if( server_index < 0 || server_index >= m_server.size() )
	{
		qCritical() << "Wrong server index!";
		return;
	}
	ServerPtr sc = m_server[server_index];

	MessagePtr m = messageAt(server_index,team_index,channel_type,channel_index,message_index);
	if(!m) {
		qCritical() << "Cant find message in channel!";
		return;
	}
	FilePtr f; // create empty FilePtr , because we request it now
	// search in requested and downloaded files same id
	for(int i = 0; i < m->m_file.size(); i++)
	{
		if(m->m_file[i]->m_id != file_id)
			continue;
		if(m->m_file[i]->m_file_status != FileStatus::FileUninitialized)
		{
			qDebug() << QStringLiteral("File [%0] already requested.").arg(file_id);
			return;
		}
		f = m->m_file[i];
		break;
	}
	if(!f) {
		f.reset(new FileContainer());
	}
	f->m_self_index = m->m_file.size();
	f->m_message_index = m->m_self_index;
	f->m_channel_type = m->m_channel_type;
	f->m_channel_index = m->m_channel_index;
	f->m_server_index = m->m_server_index;
	f->m_id = file_id;
	m->m_file.push_back(f);

	// first look file info in filesystem {conf_dir}/{server_dir}/files/{file_id}/file.json
	if( f->load_json( sc->m_data_path ) )
	{
		if( f->m_file_type == FileImage || f->m_file_type == FileAnimatedImage )
		{// check for bug (files info saved in harbour-mattermost < 0.1.3  saved withoud image_size)
			if( f->m_image_size.width() <= 0 || f->m_image_size.height() <=0 )
			{
				f->m_file_status = FileStatus::FileRequested;
				// need get file_info again
				if( m_settings->debug() )
					qDebug() << QStringLiteral("File id[%0] info requesteg again, because of wrong image_size parameter").arg(f->m_id);
			}
		}
		else
		{
			if( m_settings->debug() )
				qInfo() << "File info.json - exists! Try search file data in local storage.";
			if( f->m_file_status == FileStatus::FileDownloaded )
			{
				emit attachedFilesChanged(m, QVector<QString>(),  QVectorInt() << MessagesModel::RoleFileInfo);
				return;
			}
		}
	}
	else
		f->m_file_status = FileStatus::FileRequested;

	//files/{file_id}/info
	QString urlString = QLatin1String("/api/v")
	        + QString::number(sc->m_api)
	        + QLatin1String("/files/")
	        + file_id
	        + QLatin1String("/info");

	QUrl url(sc->m_url);
	url.setPath(url.path() + urlString);
	QNetworkRequest request;

	request.setUrl(url);
	request_set_headers(request,sc);
	request_urlencoded(request);

	QNetworkReply *reply = m_networkManager->get(request);
	reply->setProperty(P_TRUST_CERTIFICATE, QVariant(sc->m_trust_cert) );
	reply->setProperty(P_REPLY_TYPE,        QVariant(ReplyType::rt_get_file_info) );
	reply->setProperty(P_SERVER_INDEX,      QVariant(server_index) );
	reply->setProperty(P_TEAM_INDEX,        QVariant(team_index) );
	reply->setProperty(P_CHANNEL_INDEX,     QVariant(channel_index) );
	reply->setProperty(P_CHANNEL_TYPE,      QVariant((int)channel_type) );
	reply->setProperty(P_MESSAGE_INDEX,     QVariant(message_index) );
	reply->setProperty(P_FILE_INDEX,        QVariant(f->m_self_index) );
	reply->setProperty(P_FILE_ID,           QVariant(f->m_id) );
}

void MattermostQt::get_file(int server_index, int team_index,
                            int channel_type, int channel_index,
                            int message_index, int file_index)
{
	MessagePtr message = messageAt(server_index,team_index,channel_type,channel_index,message_index);
	if(!message)
	{
		qCritical() << QString("Message (si:%0,ti:%1,cht:%2,chi:%3,mi:%4) not found")
		               .arg(server_index)
		               .arg(team_index)
		               .arg(channel_type == ChannelPublic?"Public":(channel_type == ChannelPrivate?"Private":"Direct"))
		               .arg(channel_index)
		               .arg(message_index);
		return;
	}
	ServerPtr sc = m_server[server_index];
	FilePtr file = message->m_file[file_index];

	//files/{file_id}/thumbnail
	QString urlString = QLatin1String("/api/v")
	        + QString::number(sc->m_api)
	        + QLatin1String("/files/")
	        + file->m_id;

	QUrl url(sc->m_url);
	url.setPath(url.path() + urlString);
	QNetworkRequest request;

	request.setUrl(url);
	request_set_headers(request,sc);
	request_urlencoded(request);

	file->m_file_status = FileStatus::FileDownloading;
	emit fileStatusChanged(file->m_id, file->m_file_status);
	QVector<QString> file_ids;
	QVectorInt roles;
	file_ids << file->m_id;
	roles << AttachedFilesModel::FileStatus;
	attachedFilesChanged(message, file_ids, roles);

	QNetworkReply *reply = m_networkManager->get(request);
	reply->setProperty(P_TRUST_CERTIFICATE, QVariant(sc->m_trust_cert) );
	reply->setProperty(P_REPLY_TYPE, QVariant(ReplyType::rt_get_file) );
	reply->setProperty(P_FILE_PTR, QVariant::fromValue<FilePtr>(file) );
	connect(reply, SIGNAL(downloadProgress(qint64,qint64)), SLOT(replyDownloadProgress(qint64,qint64)));
}

void MattermostQt::post_file_upload(int server_index, int team_index, int channel_type,
                                    int channel_index, QString file_path)
{
	ChannelPtr channel = channelAt(server_index, team_index, channel_type, channel_index);
	if(!channel)
		return;
	ServerPtr sc = m_server[server_index];

	QFile *file = new QFile(file_path);
	if(!file->open(QIODevice::ReadOnly))
	{
		qWarning() << "Can not open File " << file_path;
		delete file;
		return;
	}

	QString urlString = QLatin1String("/api/v")
	        + QString::number(sc->m_api)
	        + QLatin1String("/files");
	QFileInfo fileinfo(*file);

	QUrlQuery query;
	query.addQueryItem("channel_id", channel->m_id);
	query.addQueryItem("filename", fileinfo.fileName());

	QUrl url(sc->m_url);
	url.setPath(url.path() + urlString);
	url.setQuery(query);

	QNetworkRequest request;
	request.setUrl(url);
	request_set_headers(request,sc);
	request_urlencoded(request);

	QHttpMultiPart *multipart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

	//	QMimeDatabase db;
	//	QMimeType type = db.mimeTypeForFile(fileinfo);

	//	QHttpPart textPart;
	//	textPart.setHeader(QNetworkRequest::ContentDispositionHeader, QString("form-data; name=\"channel_id\""));
	//	textPart.setBody(channel->m_id.toUtf8());
	//	multipart->append(textPart);

	//	QHttpPart filePart;
	//	QString mtype = type.name();
	////	filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant(type.name()));
	//	filePart.setHeader(QNetworkRequest::ContentTypeHeader, QString("application/octet-stream") );
	//	filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
	//	                   QString("form-data; name=\"files\"; filename=\"%0\"").arg(fileinfo.fileName()) );
	//	filePart.setBodyDevice(file);
	//	multipart->append(filePart);


	QNetworkReply *reply = m_networkManager->post(request,file);
	file->setParent(reply); // we cannot delete the file now, so delete it with the multiPart
	multipart->setParent(reply);// delete multipart with reply
	reply->setProperty(P_TRUST_CERTIFICATE, QVariant(sc->m_trust_cert) );
	reply->setProperty(P_REPLY_TYPE, QVariant(ReplyType::rt_post_file_upload) );
	reply->setProperty(P_FILE_PATH, file_path );
	reply->setProperty(P_CHANNEL_PTR, QVariant::fromValue<ChannelPtr>(channel) );
	connect(reply, SIGNAL( uploadProgress(qint64,qint64) ), SLOT(replyUploadProgress(qint64,qint64)));
}

void MattermostQt::post_send_message(QString message, int server_index, int team_index, int channel_type,
                                     int channel_index, QString root_id)
{
	ChannelPtr channel = channelAt(server_index,team_index,channel_type,channel_index);
	if(!channel)
	{
		qCritical() << "Cannot find channel!!!";
		return;
	}
	ServerPtr sc = m_server[server_index];
	//files/{file_id}/thumbnail
	QString urlString = QLatin1String("/api/v")
	        + QString::number(sc->m_api)
	        + QLatin1String("/posts");

	QUrl url(sc->m_url);
	url.setPath(url.path() + urlString);
	QNetworkRequest request;

	request.setUrl(url);
	request_set_headers(request,sc);
	request_json(request);

	QJsonDocument json;
	QJsonObject root;
	QJsonArray files;

	QList<FilePtr>::iterator
	        it = sc->m_unattached_file.begin(),
	        end = sc->m_unattached_file.end();
	while(it != end)
	{
		FilePtr f = *it;
		ChannelPtr c = channelAt(f->m_server_index,f->m_team_index,f->m_channel_type,f->m_channel_index);
		if(!c) {
			it++;
			continue;
		}
		if(c->m_id == channel->m_id)
		{
			files.append(f->m_id);
			it = sc->m_unattached_file.erase(it);
			sc->m_sended_files.append(f);
		}
	}

	root["channel_id"] = channel->m_id;
	root["message"] = message;
	root["file_ids"] = files;
	root["props"] = QJsonObject();
	if( !root_id.isEmpty() )
		root["root_id"] = root_id;
	json.setObject(root);

	QNetworkReply *reply = m_networkManager->post(request, json.toJson());
	reply->setProperty(P_TRUST_CERTIFICATE, QVariant(sc->m_trust_cert) );
	reply->setProperty(P_REPLY_TYPE, QVariant(ReplyType::rt_post_send_message) );
	reply->setProperty(P_SERVER_INDEX, QVariant(server_index) );
	reply->setProperty(P_TEAM_INDEX, QVariant(team_index) );
	reply->setProperty(P_CHANNEL_INDEX, QVariant(channel_index) );
	reply->setProperty(P_CHANNEL_TYPE, QVariant((int)channel_type) );
}

void MattermostQt::delete_message(int server_index, int team_index, int channel_type, int channel_index, int message_index, QString message_id)
{
	ChannelPtr channel = channelAt(server_index, team_index,channel_type,channel_index);
	if(!channel)
		return;
	ServerPtr sc = m_server[server_index];
	int search_from_index = 0;
	int search_increment = 1;
	if( message_index <0 || message_index >= channel->m_message.size() )
	{
		if(message_id.isEmpty())
			return;
		if( message_index >= channel->m_message.size() )
		{
			search_from_index = channel->m_message.size() - 1;
			search_increment = -1;
		}
		message_index = -1;
	}
	for(int i = search_from_index; i >= 0; i+=search_increment )
	{
		if( channel->m_message[i]->m_id == message_id )
		{
			message_index = i;
			break;
		}
	}
	if( message_index == -1 )
	{
		qCritical() << "Seems, message already deleted! id("<< message_id <<")";
	}
	MessagePtr message = channel->m_message[message_index];

	QString urlString = QLatin1String("/api/v")
	        + QString::number(sc->m_api)
	        + QLatin1String("/posts/")
	        + message->m_id;

	QUrl url(sc->m_url);
	url.setPath(url.path() + urlString);
	QNetworkRequest request;

	request.setUrl(url);
	request_set_headers(request,sc);
	request_urlencoded(request);

	//	QNetworkReply *reply = m_networkManager->post(request, json.toJson());
	QNetworkReply *reply = m_networkManager->deleteResource(request);
	reply->setProperty(P_TRUST_CERTIFICATE, QVariant(sc->m_trust_cert) );
	reply->setProperty(P_REPLY_TYPE, QVariant(ReplyType::rt_delete_message) );
	reply->setProperty(P_MESSAGE_PTR, QVariant::fromValue<MessagePtr>(message) );
}

void MattermostQt::put_message_edit(QString text, int server_index, int team_index, int channel_type, int channel_index, int message_index)
{
	ChannelPtr channel = channelAt(server_index, team_index,channel_type,channel_index);
	if(!channel)
		return;
	ServerPtr sc = m_server[server_index];
	MessagePtr message = channel->m_message[message_index];

	QString urlString = QLatin1String("/api/v")
	        + QString::number(sc->m_api)
	        + QLatin1String("/posts/")
	        + message->m_id
	        + QLatin1String("/patch");

	QUrl url(sc->m_url);
	url.setPath(url.path() + urlString);
	QNetworkRequest request;

	request.setUrl(url);
	request_set_headers(request,sc);
	request_json(request);

	//	"message": "string",
	//	"file_ids": [ ],
	//	"has_reactions": true,
	//	"props": "string"
	QJsonDocument json;
	QJsonObject root;
	root["message"] = text;
	json.setObject(root);

	QNetworkReply *reply = m_networkManager->put(request, json.toJson());
	reply->setProperty(P_TRUST_CERTIFICATE, QVariant(sc->m_trust_cert) );
	reply->setProperty(P_REPLY_TYPE, QVariant(ReplyType::rt_put_message_edit) );
	reply->setProperty(P_MESSAGE_PTR, QVariant::fromValue<MessagePtr>(message) );
}

void MattermostQt::post_channel_view(int server_index, int team_index, int channel_type, int channel_index)
{
	///channels/members/{user_id}/view
	ServerPtr sc = m_server[server_index];
	QString urlString = QLatin1String("/api/v")
	        + QString::number(sc->m_api)
	        + QLatin1String("/channels/members/")
	        + sc->m_user_id
	        + QLatin1String("/view");
	ChannelPtr channel;

	if(channel_type == ChannelType::ChannelDirect)
	{
		channel = sc->m_direct_channels[channel_index];
	}
	else {
		TeamPtr tc = sc->m_teams[team_index];
		QVector<ChannelPtr> *channels = nullptr;
		if(channel_type == ChannelType::ChannelPublic)
			channels = &tc->m_public_channels;
		else if(channel_type == ChannelType::ChannelPrivate)
			channels = &tc->m_private_channels;
		if(!channels)
			return;
		channel = channels->at(channel_index);
	}

	QUrl url(sc->m_url);
	url.setPath(url.path() + urlString);
	QNetworkRequest request;

	request.setUrl(url);
	request_set_headers(request,sc);
	request_json(request);

	//{
	//	"channel_id": "string", //Required
	//	"prev_channel_id": "string"
	//}
	QJsonDocument json;
	QJsonObject root;
	root["channel_id"] = channel->m_id;
	json.setObject(root);

	QNetworkReply *reply = m_networkManager->post(request, json.toJson());
	reply->setProperty(P_TRUST_CERTIFICATE, QVariant(sc->m_trust_cert) );
	reply->setProperty(P_REPLY_TYPE, QVariant(ReplyType::rt_post_channel_view) );
	reply->setProperty(P_CHANNEL_PTR, QVariant::fromValue<ChannelPtr>(channel) );
}

void MattermostQt::get_channel_unread(int server_index, int team_index, int channel_type, int channel_index)
{
	ChannelPtr channel = channelAt(server_index,team_index,channel_type,channel_index);
	get_channel_unread(channel);
}

void MattermostQt::get_channel_unread(MattermostQt::ChannelPtr channel)
{
	if( !channel ) {
		qCritical() << QStringLiteral("Wrong channels unread requset data") ;
	}

	ServerPtr sc = m_server[channel->m_server_index];

	if( !sc) {
		qCritical() << QStringLiteral("Wrong server index in get channels unread request. Bad channel PTR") ;
	}

	qDebug() << QStringLiteral("Request: Get channel unread for %1 (%2)").arg(channel->m_display_name).arg(channel->m_id);

	QString urlString = QLatin1String("/api/v")
	        + QString::number(sc->m_api)
	        + QLatin1String("/users/")
	        + sc->m_user_id
	        + QLatin1String("/channels/")
	        + channel->m_id
	        + QLatin1String("/unread");

	QUrl url(sc->m_url);
	url.setPath(url.path() + urlString);
	QNetworkRequest request;

	request.setUrl(url);
	request_set_headers(request,sc);
	request_urlencoded(request);

	if(sc->m_trust_cert && !sc->m_cert.isNull() )
		request.setSslConfiguration(sc->m_cert);

	QNetworkReply *reply = m_networkManager->get(request);
	reply->setProperty(P_REPLY_TYPE, QVariant(ReplyType::rt_get_channel_unread) );
	reply->setProperty(P_SERVER_INDEX, QVariant(channel->m_server_index) );
	reply->setProperty(P_TEAM_INDEX, QVariant(channel->m_team_index) );
	reply->setProperty(P_CHANNEL_TYPE, QVariant(channel->m_type) );
	reply->setProperty(P_CHANNEL_INDEX, QVariant(channel->m_self_index) );
	reply->setProperty(P_CHANNEL_PTR, QVariant::fromValue<ChannelPtr>(channel) );
}

void MattermostQt::get_user_image(int server_index, int user_index)
{
	if( server_index < 0 || server_index >= m_server.size() )
		return;
	ServerPtr sc = m_server[server_index];
	if( user_index < 0 || user_index >= sc->m_user.size() )
		return;
	UserPtr user = sc->m_user[user_index];

	QString path = sc->m_cache_path +
	        QString("/users/") +
	        user->m_id +
	        QString("/image.png");
	// check if user has image
	QFile user_image(path);

	if( user_image.exists() ) {
		user->m_image_path = path;
		return;
	}

	QString urlString = QLatin1String("/api/v")
	        + QString::number(sc->m_api)
	        + QLatin1String("/users/")
	        + user->m_id
	        + QLatin1String("/image");

	QUrl url(sc->m_url);
	url.setPath(url.path() + urlString);
	QNetworkRequest request;

	request.setUrl(url);
	request_set_headers(request,sc);
	request_urlencoded(request);

	if(sc->m_trust_cert)
		request.setSslConfiguration(sc->m_cert);

	QNetworkReply *reply = m_networkManager->get(request);
	reply->setProperty(P_REPLY_TYPE, QVariant(ReplyType::rt_get_user_image) );
	reply->setProperty(P_SERVER_INDEX, QVariant(server_index) );
	reply->setProperty(P_USER_INDEX, QVariant(user_index) );
}

void MattermostQt::get_user_info(int server_index, QString userId, int team_index)
{
	bool direct_channel = (bool)(team_index == GET_USER_INFO_direct_channel);
	if( server_index < 0 || server_index >= m_server.size() )
		return;
	ServerPtr sc = m_server[server_index];

	// first check if user requested allready
	for(int u = 0; u < sc->m_requested_users.size(); u++ )
	{
		if( sc->m_requested_users[u] == userId )
			return;
	}

	QString urlString = QLatin1String("/api/v")
	        + QString::number(sc->m_api)
	        + QLatin1String("/users/")
	        + userId;

	QUrl url(sc->m_url);
	url.setPath(url.path() + urlString);
	QNetworkRequest request;

	request.setUrl(url);
	request_set_headers(request,sc);
	request_urlencoded(request);

	if(sc->m_trust_cert)
		request.setSslConfiguration(sc->m_cert);

	QNetworkReply *reply = m_networkManager->get(request);
	reply->setProperty(P_REPLY_TYPE, QVariant(ReplyType::rt_get_user_info) );
	reply->setProperty(P_SERVER_INDEX, QVariant(server_index) );
	reply->setProperty(P_TEAM_INDEX, QVariant(team_index) );
	reply->setProperty(P_DIRECT_CHANNEL, QVariant(direct_channel) );
	reply->setProperty(P_USER_ID, QVariant(userId));
}

void MattermostQt::get_teams_unread(int server_index)
{
	if( server_index < 0 || server_index >= m_server.size() )
		return;
	get_teams_unread(m_server[server_index]);
}

//void MattermostQt::get_users(int server_index)
//{
//	if( server_index < 0 || server_index >= m_server.size() )
//		return;
//	ServerPtr sc = m_server[server_index];

//	// first check if user requested allready
//	for(int u = 0; u < sc->m_requested_users.size(); u++ )
//	{
//		if( sc->m_requested_users[u] == userId )
//			return;
//	}

//	QString urlString = QLatin1String("/api/v")
//	        + QString::number(sc->m_api)
//	        + QLatin1String("/users/")
//	        + userId;

//	QUrl url(sc->m_url);
//	url.setPath(url.path() + urlString);
//	QNetworkRequest request;

//	request.setUrl(url);
//	request_set_headers(request,sc);
//	request_urlencoded(request);

//	if(sc->m_trust_cert)
//		request.setSslConfiguration(sc->m_cert);

//	QNetworkReply *reply = m_networkManager->get(request);
//	reply->setProperty(P_REPLY_TYPE, QVariant(ReplyType::rt_get_user_info) );
//	reply->setProperty(P_SERVER_INDEX, QVariant(server_index) );
//	reply->setProperty(P_TEAM_INDEX, QVariant(team_index) );
//	reply->setProperty(P_DIRECT_CHANNEL, QVariant(direct_channel) );
//	reply->setProperty(P_USER_ID, QVariant(userId));
//}

void MattermostQt::get_post(int server_index, QString post_id, MattermostQt::MessagePtr message)
{
	ServerPtr sc;
	if( !message.isNull() && !message->m_root_id.isEmpty() )
	{
		post_id = message->m_root_id;
		server_index = message->m_server_index;
		// try search post in channel
		ChannelPtr channel = channelAt(server_index,message->m_team_index,message->m_channel_type,message->m_channel_index);
		if( channel )
		{
			if(message->m_self_index >= 0 && message->m_self_index < channel->m_message.size() )
			{// this should be faster, becuse answered message could near
				for(int i = message->m_self_index; i > 0; i--)
				{
					if( channel->m_message[i]->m_id == message->m_root_id )
					{
						message->m_root_ptr = channel->m_message[i];
						message->m_root_ptr->m_thread_messages.append(message);
						break;
					}
				}
			}
			else for(int i =0; i < channel->m_message.size(); i++)
			{
				if( channel->m_message[i]->m_id == message->m_root_id )
				{
					message->m_root_ptr = channel->m_message[i];
					message->m_root_ptr->m_thread_messages.append(message);
					break;
				}
			}

			if( message->updateRootMessage(this) )
			{
				QList<MessagePtr> mlist;
				mlist.push_back(message);
				emit messageUpdated(mlist);
				return;
			}
		}
	}
	if ( server_index >=0 && server_index < m_server.size() )
	{
		sc = m_server[server_index];
	}
	else {
		qWarning() << "Wrong server index";
		return;
	}

	QString urlString = QLatin1String("/api/v")
	        + QString::number(sc->m_api)
	        + QLatin1String("/posts/")
	        + post_id;

	QUrl url(sc->m_url);
	url.setPath(url.path() + urlString);
	QNetworkRequest request;

	request.setUrl(url);
	request_set_headers(request,sc);
	request_urlencoded(request);

	if(sc->m_trust_cert) {
		request.setSslConfiguration(sc->m_cert);
	}

	QNetworkReply *reply = m_networkManager->get(request);
	reply->setProperty(P_REPLY_TYPE, QVariant(ReplyType::rt_get_post) );
	reply->setProperty(P_SERVER_INDEX, QVariant(server_index) );
	//	reply->setProperty(P_TEAM_INDEX, QVariant(team_index) );
	//	reply->setProperty(P_CHANNEL_TYPE, QVariant((int)channel->m_type) );
	//	reply->setProperty(P_CHANNEL_INDEX, QVariant(channel_index) );
	reply->setProperty(P_MESSAGE_PTR, QVariant::fromValue<MessagePtr>(message) );
}

void MattermostQt::get_posts(int server_index, int team_index, int channel_type, int channel_index )
{
	ChannelPtr channel = channelAt(server_index, team_index, channel_type, channel_index);
	if( channel.isNull() )
		return;
	ServerPtr sc = m_server[server_index];

	//before send a requset, we chek if channel already have posts
	if(!channel->m_message.isEmpty())
	{// then we just send signal with messages
		emit messagesAdded(channel);
		return;
	}

	// request url channels/{channel_id}/posts?param1=val1&paramN=valN
	/*
	 * page       string  "0"    page to select
	 * per_page   string  "60"   number of posts
	 * since      int      -     time
	 * before     string   -     post id
	 * after      string   -     post id
	 */
	//	QString per_page("\"20\"");
	QString urlString = QLatin1String("/api/v")
	        + QString::number(sc->m_api)
	        + QLatin1String("/channels/")
	        + channel->m_id
	        + QLatin1String("/posts");
	//	        + per_page;

	QUrl url(sc->m_url);
	url.setPath(url.path() + urlString);
	QNetworkRequest request;

	request.setUrl(url);
	request_set_headers(request,sc);
	request_urlencoded(request);

	if(sc->m_trust_cert) {
		request.setSslConfiguration(sc->m_cert);
	}

	QNetworkReply *reply = m_networkManager->get(request);
	reply->setProperty(P_REPLY_TYPE, QVariant(ReplyType::rt_get_posts) );
	reply->setProperty(P_SERVER_INDEX, QVariant(server_index) );
	reply->setProperty(P_TEAM_INDEX, QVariant(team_index) );
	reply->setProperty(P_CHANNEL_TYPE, QVariant((int)channel->m_type) );
	reply->setProperty(P_CHANNEL_INDEX, QVariant(channel_index) );
}

void MattermostQt::get_posts_before(int server_index, int team_index,
                                    int channel_index, int channel_type)
{
	MessagePtr before;
	if( server_index < 0 || server_index >= m_server.size() )
		return;
	ServerPtr sc = m_server[server_index];
	ChannelPtr channel = channelAt(server_index,team_index,channel_type,channel_index);
	if(!channel)
		return;

	if(channel->m_message.isEmpty())
	{
		get_posts(server_index,team_index,channel_type,channel_index);
		return;
	}
	// request url channels/{channel_id}/posts?param1=val1&paramN=valN
	/*
	 * page       string  "0"    page to select
	 * per_page   string  "60"   number of posts
	 * since      int      -     time
	 * before     string   -     post id
	 * after      string   -     post id
	 */
	//	if(before.isNull())
	before = channel->m_message[0];

	QString urlString = QLatin1String("/api/v")
	        + QString::number(sc->m_api)
	        + QLatin1String("/channels/")
	        + channel->m_id
	        + QLatin1String("/posts");

	QUrlQuery query;
	query.addQueryItem("before", before->m_id);
	QUrl url(sc->m_url);
	url.setPath(url.path() + urlString);
	url.setQuery(query.query());
	QNetworkRequest request;

	request.setUrl(url);
	request_set_headers(request,sc);
	request_urlencoded(request);

	if(sc->m_trust_cert) {
		request.setSslConfiguration(sc->m_cert);
	}

	QNetworkReply *reply = m_networkManager->get(request);
	reply->setProperty(P_REPLY_TYPE, QVariant(ReplyType::rt_get_posts_before) );
	reply->setProperty(P_SERVER_INDEX, QVariant(server_index) );
	reply->setProperty(P_TEAM_INDEX, QVariant(team_index) );
	reply->setProperty(P_CHANNEL_TYPE, QVariant((int)channel->m_type) );
	reply->setProperty(P_CHANNEL_INDEX, QVariant(channel_index) );
	reply->setProperty(P_MESSAGE_INDEX, QVariant(before->m_self_index) );
}

void MattermostQt::post_users_status(int server_index)
{
	if( server_index < 0 || server_index >= m_server.size() )
		return;
	ServerPtr sc = m_server[server_index];

	QString urlString = QLatin1String("/api/v")
	        + QString::number(sc->m_api)
	        + QLatin1String("/users/status/ids");

	QUrl url(sc->m_url);
	url.setPath(url.path() + urlString);
	QNetworkRequest request;

	if(sc->m_user.isEmpty())
	{
		//m_user_status_timer.start(1000);
		return;
	}

	QJsonDocument json;
	QJsonArray ids;

	for(int i = 0; i < sc->m_user.size(); i++)
	{
		ids.append(sc->m_user[i]->m_id);
	}
	json.setArray(ids);

	request.setUrl(url);
	request_set_headers(request,sc);
	request_json(request);

	if(sc->m_trust_cert)
		request.setSslConfiguration(sc->m_cert);

	QNetworkReply *reply = m_networkManager->post(request,json.toJson());
	reply->setProperty(P_REPLY_TYPE, QVariant(ReplyType::rt_post_users_status) );
	reply->setProperty(P_SERVER_INDEX, QVariant(server_index) );
}

void MattermostQt::post_create_reaction(int server_index, const QString &post_id, const QString &emoji_name) const
{
	if( server_index < 0 || server_index >= m_server.size() )
		return;

	qDebug() << QStringLiteral("Add reaction (%0) to post id(%1).").arg(emoji_name).arg(post_id);

	ServerPtr sc = m_server[server_index];

	QString urlString = QLatin1String("/api/v")
	        + QString::number(sc->m_api)
	        + QLatin1String("/reactions");

	QUrl url(sc->m_url);
	url.setPath(url.path() + urlString);
	QNetworkRequest request;

	QJsonDocument json;
	QJsonObject json_object;
	json_object["user_id"]    = sc->m_user_id;
	json_object["post_id"]    = post_id;
	json_object["emoji_name"] = emoji_name;
	json_object["create_at"]  = 0; //QDateTime::currentDate().toMilliSeconds();
	json.setObject(json_object);
	request.setUrl(url);
	request_set_headers(request,sc);
	request_json(request);

	if(sc->m_trust_cert)
		request.setSslConfiguration(sc->m_cert);

	QNetworkReply *reply = m_networkManager->post(request,json.toJson());
	reply->setProperty(P_REPLY_TYPE, QVariant(ReplyType::rt_post_create_reaction) );
	reply->setProperty(P_SERVER_INDEX, QVariant(server_index) );

	SettingsContainer::getInstance()->addUsedReaction(emoji_name);
}

void MattermostQt::delete_reaction(int server_index, const QString &post_id, const QString &emoji_name) const
{
	if( server_index < 0 || server_index >= m_server.size() )
	{
		qCritical() << QStringLiteral("Wrong server index");
		return;
	}

	qDebug() << QStringLiteral("Remove reaction (%0) from post id(%1).").arg(emoji_name).arg(post_id);

	ServerPtr sc = m_server[server_index];

	QString urlString = QLatin1String("/api/v")
	        + QString::number(sc->m_api)
	        + QStringLiteral("/users/%0/posts/%1/reactions/%2")
	        .arg(sc->m_user_id)
	        .arg(post_id)
	        .arg(emoji_name);

	QUrl url(sc->m_url);
	url.setPath(url.path() + urlString);
	QNetworkRequest request;

	request.setUrl(url);
	request_set_headers(request,sc);
	request_urlencoded(request);

	//	QNetworkReply *reply = m_networkManager->post(request, json.toJson());
	QNetworkReply *reply = m_networkManager->deleteResource(request);
	reply->setProperty(P_TRUST_CERTIFICATE, QVariant(sc->m_trust_cert) );
	reply->setProperty(P_REPLY_TYPE, QVariant(ReplyType::rt_delete_reaction) );
	reply->setProperty(P_SERVER_INDEX, QVariant(server_index) );
}

QString MattermostQt::user_id(int server_index) const
{
	return m_server[server_index]->m_user_id;
}

bool MattermostQt::user_role(int server_index, int user_index, int role) const
{
	if( server_index < 0 || server_index >= m_server.size() ||
	        role < 0 || role >= UserSystemRolesCount )
		return false;
	ServerPtr server = m_server[server_index];
	if( user_index == GET_USER_INFO_current_user ) {
		if(!server->m_current_user) {
			return false;
		}
		return server->m_current_user->m_roles[role];
	}
	else if( user_index < 0 || user_index >= server->m_user.size() )
		return false;
	return server->m_user[user_index]->m_roles[role];
}

QString MattermostQt::getChannelName(int server_index, int team_index, int channel_type, int channel_index)
{
	ChannelPtr channel = channelAt(server_index, team_index, channel_type, channel_index);
	if(channel)
		return channel->m_display_name;
	return QString();
}

QString MattermostQt::getChannelId(int server_index, int team_index, int channel_type, int channel_index)
{
	ChannelPtr channel = channelAt(server_index, team_index, channel_type, channel_index);
	if(channel)
		return channel->m_id;
	return QString();
}

QString MattermostQt::getTeamId(int server_index, int team_index)
{
	TeamPtr team = teamAt(server_index, team_index);
	if(!team)
		return QString::Null();
	return team->m_id;
}

QString MattermostQt::getTeamName(int server_index, int team_index)
{
	TeamPtr team = teamAt(server_index, team_index);
	if(!team)
		return QString::Null();
	return team->m_display_name;
}

void MattermostQt::notificationActivated(int server_index, int team_index, int channel_type, int channel_index)
{
	//
}

QString MattermostQt::parseMD(const QString &input) const
{
	if(!m_mdParser)
		return QString::Null();
	return m_mdParser->parse(input);
}

QString formatSize(qint64 size) {
	QStringList units = {
	    QObject::tr("Bytes"),
	    QObject::tr("KB"),
	    QObject::tr("MB"),
	    QObject::tr("GB"),
	    QObject::tr("TB"),
	    QObject::tr("PB")
	};
	int i;
	double outputSize = size;
	for(i=0; i<units.size()-1; i++) {
		if(outputSize<1024) break;
		outputSize= outputSize/1024;
	}
	return QString("%0 %1").arg(outputSize, 0, 'f', 2).arg(units[i]);
}

qint64 dirSize(QString dirPath) {
	qint64 size = 0;
	QDir dir(dirPath);
	//calculate total size of current directories' files
	QDir::Filters fileFilters = QDir::Files|QDir::System|QDir::Hidden;
	for(QString filePath : dir.entryList(fileFilters)) {
		QFileInfo fi(dir, filePath);
		size+= fi.size();
	}
	//add size of child directories recursively
	QDir::Filters dirFilters = QDir::Dirs|QDir::NoDotAndDotDot|QDir::System|QDir::Hidden;
	for(QString childDirPath : dir.entryList(dirFilters))
		size+= dirSize(dirPath + QDir::separator() + childDirPath);
	return size;
}

QString MattermostQt::cacheSize() const
{
	qint64 sz = 0;
	for( int i = 0; i < m_server.size(); i++ )
	{
		sz += dirSize(m_server[i]->m_cache_path);
	}
	return formatSize(sz);
}

void MattermostQt::clearCache()
{
	for( int i = 0; i < m_server.size(); i++ )
	{
		QDir dir(m_server[i]->m_cache_path);
		dir.removeRecursively();
		for(int f = 0; f < m_server[i]->m_file.size(); f++)
		{
			FilePtr file = m_server[i]->m_file[f];
			if(file->m_file_type != FileImage ||
			        file->m_file_type != FileAnimatedImage )
				continue;
			// TODO check if file is on local storage
			file->m_file_status = FileRemote;
			file->m_thumb_path = QString::Null();
			file->m_preview_path = QString::Null();
			//			emit fileStatusChanged(m_server[i]->m_file[f]->m_id, FileRemote); // not need
			MessagePtr m = messageAt(i, file->m_team_index, file->m_channel_type, file->m_channel_index, file->m_message_index);
			if(!m)
				continue;
			//			QVector<QString>() << file->m_id;
			QVectorInt roles;
			roles << AttachedFilesModel::FileThumbnailPath
			      << AttachedFilesModel::FilePreviewPath;
			emit attachedFilesChanged(m, QVector<QString>() << file->m_id, roles);
		}
	}
}

bool MattermostQt::isImageFileInGallery(int server_index, int file_sc_index)
{
	if( server_index < 0 || server_index >= m_server.size() )
		return false;
	if( file_sc_index < 0 || file_sc_index >= m_server[server_index]->m_file.size() )
		return false;
	ServerPtr server = m_server[server_index];
	FilePtr file = server->m_file[file_sc_index];

	return !file->is_file_in_cache(server->m_cache_path);
}

QString MattermostQt::saveImageFileToGallery(int server_index, int file_sc_index)
{
	if( server_index < 0 || server_index >= m_server.size() )
		return  QString();
	if( file_sc_index < 0 || file_sc_index >= m_server[server_index]->m_file.size() )
		return  QString();
	FilePtr file = m_server[server_index]->m_file[file_sc_index];
	QString new_file_path = m_pictures_path + QDir::separator();

	QDir dir(new_file_path);
	if( !dir.exists() )
		if( !dir.mkpath(new_file_path) )
			return QString();

	new_file_path += file->m_name;
	if( QFile::rename( file->m_file_path, new_file_path ) ) {
		file->m_file_path = new_file_path;
		file->m_is_in_cache = 0;
		file->save_json( m_server[file->m_server_index]->m_data_path );

		emit onImageFileSavedToGallery(server_index, file_sc_index);

		MessagePtr m = messageAt(file->m_server_index,
		                         file->m_team_index,
		                         file->m_channel_type,
		                         file->m_channel_index,
		                         file->m_message_index );
		if(m) {
			emit attachedFilesChanged(m, QVector<QString>() << file->m_id,
			                          QVectorInt() << AttachedFilesModel::FilePath );
		}
	}
	else
		return QString();
	return new_file_path;
}

QString MattermostQt::getUserName(int server_index, int user_index)
{
	if(server_index < 0 || server_index >= m_server.size()
	        || user_index < 0 || user_index >= m_server[server_index]->m_url.size() )
		return QString();
	return m_server[server_index]->m_user[user_index]->m_username;
}

void MattermostQt::get_teams_unread(MattermostQt::ServerPtr server)
{
	// request url users/{user_id}/teams/unread
	QString urlString = QLatin1String("/api/v")
	        + QString::number(server->m_api)
	        + QLatin1String("/users/me/teams/unread");


	QUrl url(server->m_url);
	url.setPath(url.path() + urlString);
	QNetworkRequest request;

	request.setUrl(url);
	request_set_headers(request,server);
	request_urlencoded(request);

	if(server->m_trust_cert)
		request.setSslConfiguration(server->m_cert);

	QNetworkReply *reply = m_networkManager->get(request);
	reply->setProperty(P_REPLY_TYPE, QVariant(ReplyType::rt_get_teams_unread) );
	reply->setProperty(P_SERVER_INDEX, QVariant(server->m_self_index) );
}

bool MattermostQt::save_settings()
{
	QJsonDocument json;
	QJsonObject object;
	QJsonArray servers;
	for(int i = 0; i < m_server.size(); i ++ )
	{
		ServerPtr sc = m_server[i];
		QJsonObject server;
		QString server_dir_path;
		//		server["id"] = (double)sc->m_selfId;
		server["user_id"] = sc->m_user_id;
		server["url"] = sc->m_url;
		server["api"] = (double)sc->m_api;
		server["token"] = sc->m_token;
		server["name"] = sc->m_display_name;
		server["trust_certificate"] = sc->m_trust_cert;
		server["enabled"] = sc->m_enabled;

		server_dir_path = QString("%0_%1").arg(i).arg(sc->m_user_id);

		sc->m_data_path = m_data_path + QDir::separator() + server_dir_path;
		sc->m_cache_path = m_cache_path + QDir::separator() + server_dir_path;

		if(sc->m_trust_cert)
		{
			QDir server_dir(sc->m_data_path);
			if(! server_dir.exists() )
				server_dir.mkpath(sc->m_data_path);
			QDir server_cache(sc->m_cache_path);
			if(! server_cache.exists() )
				server_cache.mkpath(sc->m_cache_path);
			QString new_ca_path = sc->m_data_path + QString("/ca.crt");
			if( QFile::copy(sc->m_ca_cert_path , new_ca_path) )
				sc->m_ca_cert_path = new_ca_path;
			QString new_cert_path = sc->m_data_path + QString("/server.crt");
			if( QFile::copy(sc->m_ca_cert_path , new_cert_path) )
				sc->m_cert_path = new_cert_path;
		}
		server["ca_cert_path"] = QString("ca.crt");
		server["cert_path"] = QString("server.crt");
		server["server_dir"] = server_dir_path;
		servers.append(server);
	}
	object["servers"] = servers;
	json.setObject(object);

	{
		QDir dir(m_data_path);
		if(!dir.exists())
			dir.mkpath(m_data_path);
		QFile jsonFile( m_data_path + QDir::separator() + QLatin1String(F_CONFIG_FILE) );
		if( jsonFile.open(QFile::WriteOnly) )
		{
			jsonFile.write(json.toJson());
			jsonFile.close();
		}
	}

	json = QJsonDocument();
	//	;
	QJsonObject settings = m_settings->asJsonObject();
	json.setObject(settings);

	{
		QDir dir(m_config_path);
		if(!dir.exists())
			dir.mkpath(m_config_path);
		QFile jsonFile( m_config_path + QDir::separator() + QLatin1String(F_SETTINGS_FILE) );
		if( !jsonFile.open(QFile::WriteOnly) )
			return false;
		jsonFile.write(json.toJson());
		jsonFile.close();
	}

	return true;
}

bool MattermostQt::load_settings()
{
	QJsonDocument json;
	QFile jsonFile( m_data_path + QDir::separator() + QLatin1String(F_CONFIG_FILE) );
	if( !jsonFile.open(QFile::ReadOnly | QFile::Text) ) {
		qWarning() << QStringLiteral("No config file found.");
		return false;
	}
	qDebug() << QStringLiteral("Read settings from config file: %0").arg(m_data_path + QDir::separator() + QLatin1String(F_CONFIG_FILE));

	QJsonParseError error;
	json = QJsonDocument::fromJson(jsonFile.readAll(), &error);
	jsonFile.close();

	if( json.isNull() || !json.isObject())
	{
		qWarning() << error.errorString();
		return false;
	}

	QJsonArray servers;
	if(json.object()["servers"].isArray())
		servers = json.object()["servers"].toArray();
	if(servers.isEmpty())
		return false;

	m_server.clear();
	for( int i = 0; i < servers.size(); i ++ )
	{
		if( !servers[i].isObject() )
			return false;
		QJsonObject object = servers[i].toObject();

		QString user_id = object["user_id"].toString();
		QString url = object["url"].toString();
		int api = (int)object["api"].toDouble();
		QString token = object["token"].toString();
		QString display_name = object["name"].toString();
		QString ca_cert_path = object["ca_cert_path"].toString();
		QString cert_path = object["cert_path"].toString();
		bool    is_server_enabled = true;
		if( object.contains("enabled") )
			is_server_enabled = object["enabled"].toBool();
		bool trust_certificate = object["trust_certificate"].toBool();

		if( user_id.isEmpty() || url.isEmpty() || token.isEmpty() )
			return false;

		// create server container
		ServerPtr server( new ServerContainer(url,token,api) );
		QString server_dir = object["server_dir"].toString("");
		if(server_dir.isEmpty())
			server_dir = QString("%0_%1").arg(m_server.size()).arg(user_id);
		server->m_data_path = m_data_path + QDir::separator() +  server_dir;
		server->m_cache_path = m_cache_path + QDir::separator() +  server_dir;
		if(server->m_data_path.isEmpty())
		{
			server->m_data_path = m_data_path + QDir::separator() + QString("%0_%1").arg(i).arg(user_id);
			server->m_cache_path = m_cache_path + QDir::separator() + QString("%0_%1").arg(i).arg(user_id);
		}
		server->m_trust_cert = trust_certificate;
		server->m_display_name = display_name;
		server->m_self_index = m_server.size();
		server->m_ca_cert_path = server->m_data_path + QDir::separator() +  ca_cert_path;
		server->m_cert_path = server->m_data_path + QDir::separator() + cert_path;
		server->m_user_id = user_id;
		server->m_ping_timer.setInterval( m_ping_server_timeout );
		qDebug() << QStringLiteral("Server[%0] (%1) set ping timer interval to %2.").arg(server->m_self_index).arg(server->m_display_name).arg(m_ping_server_timeout);
		server->m_enabled =is_server_enabled;
		m_server.append(server);
		get_login(server);
	}
	// load settings
	if(m_settings)
	{
		QFile jsonFile( m_config_path + QDir::separator() + QLatin1String(F_SETTINGS_FILE) );
		if( !jsonFile.open(QFile::ReadOnly | QFile::Text) )
			return false;

		QJsonParseError error;
		json = QJsonDocument::fromJson(jsonFile.readAll(), &error);
		jsonFile.close();

		if( json.isNull() || !json.isObject())
		{
			qWarning() << error.errorString();
			return false;
		}

		m_settings->fromJsonObject(json.object());
	}
	else {
		qCritical() << "Can't load settings from config folder";
	}
	return true;
}

void MattermostQt::prepare_direct_channel(int server_index, int channel_index)
{
	ChannelPtr ct = m_server[server_index]->m_direct_channels[channel_index];
	ServerPtr sc = m_server[server_index];
	/** in name we have two ids, separated with '__' */
	QString ch_name = ct->m_name;
	QString user_id = m_server[server_index]->m_user_id;
	bool self_chat = false;
	ch_name = ch_name.replace(QRegExp(QString("(%0|__)").arg(user_id)),"");
	if(ch_name.isEmpty())
		self_chat = true;
	else // not self chat
		user_id = ch_name;

	// first search in cached users
	for(int i = 0; i < sc->m_user.size(); i++ )
	{
		if( sc->m_user[i]->m_id.compare(user_id) == 0 )
		{
			UserPtr user = sc->m_user[i];
			QString displayname = user->m_username;
			if( SettingsContainer::getInstance()->userNameDisplay() == SettingsContainer::UserNameFull )
			{
				if( !user->m_first_name.isEmpty() || !user->m_last_name.isEmpty() )
				{
					displayname = QStringLiteral("%0 %1").arg(user->m_first_name).arg(user->m_last_name);
				}
			}
			else if( SettingsContainer::getInstance()->userNameDisplay() == SettingsContainer::UserNameNickname ) {
				if( !user->m_nickname.isEmpty() )
					displayname = user->m_nickname;
			}
			if(self_chat)
				sc->m_direct_channels[channel_index]->m_display_name = displayname
				        + QLatin1String(" ") // for right translation, if someone forgot about space before '(you)'
				        + QObject::tr("(you)");
			else
				sc->m_direct_channels[channel_index]->m_display_name = displayname;
			sc->m_direct_channels[channel_index]->m_dc_user_index = user->m_self_index;
			QVectorInt roles;
			roles << ChannelsModel::DisplayName
			      << ChannelsModel::AvatarPath
			      << ChannelsModel::RoleUserFullName
			      << ChannelsModel::RoleUserNickname;
			emit updateChannel(ct, roles);
			return;
		}
	}
	// send request for user credentials first
	get_user_info(sc->m_self_index, user_id, GET_USER_INFO_direct_channel);
}

void MattermostQt::prepare_user_index(int server_index, MattermostQt::MessagePtr message, int tean_index)
{
	ServerPtr sc;
	if( server_index < 0 || server_index >= m_server.size() )
		return;
	sc = m_server[server_index];
	message->m_user_index = -1;
	if( message->m_user_id.isEmpty() )
	{// system message
		qDebug() << "Seems its system message";
		return;
	}
	//	else if( message->m_user_id.compare(sc->m_user_id) == 0 )
	//	{// my message
	//		message->m_user_index = -1;
	//		return;
	//	}
	else // other users messages
		for(int k = 0; k < sc->m_user.size(); k++)
		{
			if( message->m_user_id.compare(sc->m_user[k]->m_id) == 0)
			{
				message->m_user_index = k;
				break;
			}
		}
	if( message->m_user_index == -1 )
	{
		sc->m_nouser_messages.append(message);
		get_user_info(server_index, message->m_user_id, tean_index);
	}
}

MattermostQt::TeamPtr MattermostQt::find_team_by_id(MattermostQt::ServerPtr sc, QString team_id) const
{
	for( int i = 0; i < sc->m_teams.size(); i++ )
	{
		if( sc->m_teams[i]->m_id == team_id )
			return sc->m_teams[i];
	}
	return TeamPtr();
}

MattermostQt::ServerPtr MattermostQt::get_server(int server_index) const
{
	if( server_index<0 || server_index >= m_server.size() )
		return ServerPtr();
	return m_server[server_index];
}

MattermostQt::TeamPtr MattermostQt::teamAt(int server_index, int team_index)
{
	if(  server_index < 0 || server_index >= m_server.size() )
	{
		qCritical() << "Wrong server index " << server_index << " servers count " << m_server.size();
		return TeamPtr();
	}
	ServerPtr server = m_server[server_index];
	if( team_index < 0 || team_index >= server->m_teams.size() )
	{
		qCritical() << QStringLiteral("Wrong team index (%0) in server (%1) with %2 teams").arg(team_index).arg(server_index).arg(server->m_teams.size());
		return TeamPtr();
	}
	return server->m_teams[team_index];
}

MattermostQt::ChannelPtr MattermostQt::channelAt(int server_index, int team_index, int channel_type, int channel_index)
{
	ChannelPtr channel;
	if( server_index < 0 || server_index >= m_server.size() )
	{
		qCritical() << "Wrong server index " << server_index << " servers count " << m_server.size();
		return channel;
	}
	if(channel_index < 0)
	{
		qCritical() << "Wrong cahnnel index";
		return channel;
	}
	ServerPtr sc = m_server[server_index];
	if(channel_type == ChannelType::ChannelDirect)
	{
		if(channel_index >= sc->m_direct_channels.size() )
		{
			qCritical() << "Channel index more than channels size " <<  sc->m_direct_channels.size();
			return channel;
		}
		channel = sc->m_direct_channels[channel_index];
	}
	else if( team_index >= 0 && team_index < sc->m_teams.size() ) {
		TeamPtr tc = sc->m_teams[team_index];

		QVector<ChannelPtr> *channels = nullptr;

		if(channel_type == ChannelType::ChannelPublic)
			channels = &tc->m_public_channels;
		else if(channel_type == ChannelType::ChannelPrivate)
			channels = &tc->m_private_channels;

		if(!channels || channel_index >= channels->size())
		{
			qCritical() << "Channel index more than channels size " <<  channels->size();
			return channel;
		}

		channel = channels->at(channel_index);
	}
	return channel;
}

MattermostQt::MessagePtr MattermostQt::messageAt(int server_index, int team_index, int channel_type, int channel_index, int message_index)
{
	if( message_index < 0 )
	{
		qDebug() << "Message index wrong!";
		return MessagePtr();
	}
	ChannelPtr c = channelAt(server_index,team_index,channel_type,channel_index);
	if(!c || message_index  >= c->m_message.size())
		return MessagePtr();

	return c->m_message[message_index];
}

MattermostQt::UserPtr MattermostQt::userAt(int server_index, int user_index)
{
	if( server_index < 0 || server_index >= m_server.size() )
		return UserPtr();
	if( user_index < 0 || user_index >= m_server[server_index]->m_user.size() )
		return UserPtr();
	return m_server[server_index]->m_user[user_index];
}



void MattermostQt::setSettingsContainer(SettingsContainer *settings)
{
	m_settings = settings;
}

SettingsContainer *MattermostQt::settings()
{
	if( !m_settings )
		m_settings = SettingsContainer::getInstance();
	return m_settings;
}

int MattermostQt::messageUnread() const
{
	int result = 0;
	for(ServerPtr server : m_server)
	{
		for(TeamPtr team : server->m_teams )
		{
			result += team->m_unread_messages + team->m_unread_mentions;
		}
		for(ChannelPtr channel : server->m_direct_channels )
		{
			result += channel->m_msg_unread + channel->m_mention_count;
		}
	}
	return result;
}

void MattermostQt::websocket_connect(ServerPtr server)
{
	qDebug() << QStringLiteral("Configure WebSocket connetion for server[%0] %1").arg(server->m_self_index).arg(server->m_display_name);
	//	QString origin( QUuid::createUuid().toString() );
	// server get us authentificztion token, time to open WebSocket!
	QSharedPointer<QWebSocket> socket(new QWebSocket( QString(), QWebSocketProtocol::VersionLatest, this));
	server->m_socket = socket;
	socket->setProperty(P_SERVER_INDEX, server->m_self_index);

	if( server->m_trust_cert )
	{
		QList<QSslError> errors;
		errors << QSslError(QSslError::CertificateUntrusted);
		errors << QSslError(QSslError::SelfSignedCertificateInChain);
		errors << QSslError(QSslError::SelfSignedCertificate);

		QFile ca_cert_file(server->m_ca_cert_path);
		QFile cert_file(server->m_cert_path);
		if(ca_cert_file.open(QIODevice::ReadOnly))
		{
			QSslCertificate ca_cert(&ca_cert_file, QSsl::Pem);
			errors << QSslError(QSslError::CertificateUntrusted, ca_cert);
			errors << QSslError(QSslError::SelfSignedCertificateInChain, ca_cert);
			errors << QSslError(QSslError::SelfSignedCertificate, ca_cert);
			ca_cert_file.close();
		}
		else
			qWarning() << tr("Cant open CA certificate file: \"%0\"").arg(server->m_ca_cert_path);
		if( cert_file.open(QIODevice::ReadOnly))
		{
			QSslCertificate cert(&cert_file, QSsl::Pem);
			errors << QSslError(QSslError::CertificateUntrusted, cert);
			errors << QSslError(QSslError::SelfSignedCertificateInChain, cert);
			errors << QSslError(QSslError::SelfSignedCertificate, cert);
			cert_file.close();
		}
		else
			qWarning() << tr("Cant open certificate file: \"%0\"").arg(server->m_cert_path);
		errors << QSslError(QSslError::CertificateUntrusted);
		errors << QSslError(QSslError::SelfSignedCertificateInChain);
		errors << QSslError(QSslError::SelfSignedCertificate);
		socket->ignoreSslErrors(errors);
	}

	connect(socket.data(), SIGNAL(connected()), SLOT(onWebSocketConnected()));
	typedef void (QWebSocket:: *sslErrorsSignal)(const QList<QSslError> &);
	connect(socket.data(), static_cast<sslErrorsSignal>(&QWebSocket::sslErrors),
	        this, &MattermostQt::onWebSocketSslError);
	connect(socket.data(), SIGNAL(error(QAbstractSocket::SocketError)),
	        SLOT(onWebSocketError(QAbstractSocket::SocketError)));

	connect(socket.data(), SIGNAL(stateChanged(QAbstractSocket::SocketState)),
	        SLOT(onWebSocketStateChanged(QAbstractSocket::SocketState)) );
	connect(socket.data(), SIGNAL(textMessageReceived(QString)),
	        SLOT(onWebSocketTextMessageReceived(QString)) );
	connect(socket.data(), SIGNAL(pong(quint64,QByteArray)),
	        SLOT(onWebSocketPong(quint64,QByteArray)) );

	QString urlString = QLatin1String("/api/v")
	        + QString::number(server->m_api)
	        + QLatin1String("/websocket");

	QString serUrl = server->m_url;
	if( serUrl.indexOf("https://") >= 0 )
		serUrl.replace("https://","wss://");
	else if( serUrl.indexOf("http://") >= 0 )
		serUrl.replace("http://","ws://");
	else
		serUrl.replace("https://","wss://")
		        .replace("http://","ws://");

	/// fix #31
	QUrl url(serUrl);
	url.setPath(url.path() + urlString);

	if( url.toString().indexOf("wss://") >= 0 && (url.port() == 80 || url.port() == -1) ) {
		url.setPort(443);
		qWarning() << QStringLiteral("Force set port to 443 for WebSocket connection");
	}

	qDebug() << QStringLiteral("Try connect to websocket: %0").arg(url.toString());
	socket->open(url);
}

void MattermostQt::reply_login(QNetworkReply *reply)
{
	ASSERT_REPLY(reply_login)

	//	 TODO here we need check if server already exists, just need update auth data
	if( reply->property(P_SERVER_INDEX).isValid() )
	{//login by token
		ServerPtr server;

		int server_id = reply->property(P_SERVER_INDEX).toInt();
		server = m_server[server_id];

		server->m_cert  = reply->sslConfiguration();
		server->m_cookie= reply->header(QNetworkRequest::CookieHeader).toString();

		QJsonDocument json = QJsonDocument::fromJson( reply->readAll() );
		if( json.isObject() )
		{
			QJsonObject object = json.object();
//			qDebug() << object;
			server->m_user_id = object["id"].toString();
			server->m_auth_data = object["auth_data"].toString();
			server->m_auth_service = object["auth_service"].toString();
			get_user_info(server_id, server->m_user_id, GET_USER_INFO_current_user);
		}
		QString server_dir = QString("%0_%1").arg(server->m_self_index).arg(server->m_user_id);
		if( server->m_data_path.isEmpty() || server->m_data_path != m_data_path + QDir::separator() +  server_dir )
			server->m_data_path = m_data_path + QDir::separator() +  server_dir;
		if( server->m_cache_path.isEmpty() || server->m_cache_path != m_cache_path + QDir::separator() +  server_dir )
			server->m_cache_path = m_cache_path + QDir::separator() +  server_dir;
		websocket_connect(server);
		// TODO add singnal server Added
		//		emit serverConnected(server->m_self_index);
		return;
	}
	// first login, or update token
	QList<QByteArray> headerList = reply->rawHeaderList();
	foreach(QByteArray head, headerList)
	{
		//qDebug() << head << ":" << reply->rawHeader(head);
		//search login token
		if( strcmp(head.data(),"Token") == 0 )
		{// yes, auth token founded!
			//add server to servers list
			ServerPtr server;
			server.reset(new ServerContainer());
			server->m_api   = reply->property(P_API).toInt();
			server->m_url   = reply->property(P_SERVER_URL).toString();
			server->m_display_name = reply->property(P_SERVER_NAME).toString();
			server->m_token = reply->rawHeader(head).data();
			server->m_bearer = bearer_string(server->m_token);
			server->m_trust_cert = reply->property(P_TRUST_CERTIFICATE).toBool();
			server->m_ca_cert_path = reply->property(P_CA_CERT_PATH).toString();
			server->m_cert_path = reply->property(P_CERT_PATH).toString();
			server->m_cert  = reply->sslConfiguration();
			server->m_cookie= reply->header(QNetworkRequest::CookieHeader).toString();
			server->m_self_index =  m_server.size();
			server->m_ping_timer.setInterval( m_ping_server_timeout );
			qDebug() << QStringLiteral("Server[%0] (%1) set ping timer interval to %2.").arg(server->m_self_index).arg(server->m_display_name).arg(m_ping_server_timeout);

			QJsonDocument json = QJsonDocument::fromJson( reply->readAll() );
			if( json.isObject() )
			{
				QJsonObject object = json.object();
				//qDebug() << object;
				server->m_user_id = object["id"].toString();
				server->m_auth_data = object["auth_data"].toString();
				server->m_auth_service = object["auth_service"].toString();
			}

			// fisrt check if server with used_id and url exists
			bool is_new_account = true;
			if( !m_server.isEmpty() )
			{
				for(int i = 0; i < m_server.size(); i++ )
				{
					if( server->m_url == m_server[i]->m_url &&
					        server->m_user_id == m_server[i]->m_user_id )
					{
						is_new_account = false;
						m_server[i]->m_token = server->m_token;
						m_server[i]->m_bearer = bearer_string(m_server[i]->m_token);
						m_server[i]->m_cookie = server->m_cookie;
						//server.reset();
						server = m_server[i];
						break;
					}
				}
			}

			if(is_new_account)
				m_server.append( server );
			get_user_info(server->m_self_index,server->m_user_id, GET_USER_INFO_current_user);
			//get_login(server);
			websocket_connect(server);

			if(is_new_account)
				emit serverAdded(server);

			return;
		}
	}
	return;
}

void MattermostQt::reply_get_teams(QNetworkReply *reply)
{
	ASSERT_REPLY(reply_get_teams)
	        QJsonDocument json;
	QByteArray rawData = reply->readAll();
	int server_index = reply->property(P_SERVER_INDEX).toInt();
	json = QJsonDocument::fromJson(rawData);

	if( !json.isEmpty() && json.isArray() ) {
		QJsonArray array = json.array();
		for( int i = 0 ; i < array.size(); i ++ )
		{
			if( array.at(i).isObject() )
			{
				QJsonObject object = array.at(i).toObject();
				TeamPtr tc(new TeamContainer(object) );
				tc->m_server_index = server_index;
				tc->m_self_index = m_server[server_index]->m_teams.size();
				m_server[server_index]->m_teams.push_back(tc);
				// TODO move to Thread all filesystem operations
				tc->save_json( m_server[server_index]->m_data_path );
				// ---------------------------------------------
				emit teamAdded(tc);
				get_team_icon(tc->m_server_index, tc->m_self_index);
			}
			else
				qDebug() << "array[" << i << "]: " << array.at(i);
		}
		get_teams_unread(m_server[server_index]);
	}
	else {
		qWarning() << "Cant parse json: " << json;
	}
}

void MattermostQt::reply_get_team(QNetworkReply *reply)
{
	QJsonDocument json = QJsonDocument::fromJson(reply->readAll());
	int server_index = reply->property(P_SERVER_INDEX).toInt();
	int team_index = reply->property(P_TEAM_INDEX).toInt();

	if( json.isObject() )
	{
		QJsonObject object = json.object();
		TeamPtr tc(new TeamContainer(object) );
		TeamPtr tc_old = m_server[server_index]->m_teams[team_index];
		if( tc->m_update_at > tc_old->m_update_at )
		{// team is updated
			//			TODO   that when the team is changed? like header and other
			qDebug() << "Need update Team info";

			///users/{user_id}/teams/unread get_teams_unread
		}
		//		tc->m_server_index = server_index;
		//		tc->m_self_index = m_server[server_index]->m_teams.size();
		//		m_server[server_index]->m_teams << tc;
	}
	else
		qWarning() << "Cant parse reply" << json;
}

void MattermostQt::reply_get_teams_unread(QNetworkReply *reply)
{
	bool is_ok;
	int server_index = reply->property(P_SERVER_INDEX).toInt(&is_ok);
	//	Q_UNUSED(server_index)
	if(!is_ok)
		return;
	if(server_index < 0 || server_index >= m_server.size())
		return;
	QJsonDocument json = QJsonDocument::fromJson(reply->readAll());
	QJsonArray array = json.array();

	if( array.isEmpty() )
	{
		qWarning() << "Wrong Json" << json;
		return;
	}
	ServerPtr server = m_server[server_index];

	//	qDebug() << reply->header(QNetworkRequest::LastModifiedHeader);

	for( int i = 0; i < array.size(); i++ )
	{
		QJsonObject object = array.at(i).toObject();
		if(object.isEmpty())
			continue;
		QString team_id = object["team_id"].toString();
		qlonglong msg_count = (qlonglong)object["msg_count"].toDouble();
		qlonglong mention_count = (qlonglong)object["mention_count"].toDouble();
		int team_index = server->get_team_index(team_id);
		if( team_index == -1)
			continue;
		TeamPtr team = server->m_teams[team_index];
		team->m_unread_messages = (int)msg_count;
		team->m_unread_mentions = (int)mention_count;
		// TODO finish teams unread mechanism
		// change signal to TeamPtr instead team_id string
		//		emit teamUnread(team_id, (int)msg_count, (int)mention_count);
		emit teamChanged(team, QVectorInt()
		                 << TeamsModel::RoleUnreadMentionCount
		                 << TeamsModel::RoleUnreadMessageCount);
		emit messageUnreadChanged();
	}
}

void MattermostQt::reply_get_team_icon(QNetworkReply *reply)
{
	bool is_ok;
	int server_index = reply->property(P_SERVER_INDEX).toInt(&is_ok);
	//	Q_UNUSED(server_index)
	if(!is_ok)
		return;
	int team_index = reply->property(P_TEAM_INDEX).toInt(&is_ok);
	if(!is_ok)
		return;
	if(server_index < 0 || server_index >= m_server.size())
		return;
	ServerPtr server = m_server[server_index];
	if(team_index < 0 || team_index >= server->m_teams.size() )
		return;
	TeamPtr team = server->m_teams[team_index];

	QByteArray replyData = reply->readAll();
	{
		QString file_path = server->m_cache_path
		        + QLatin1String("/teams/")
		        + team->m_id;
		QDir dir;
		if( QFile::exists(file_path + QLatin1String("/image.png")) )
		{
			qWarning() << QStringLiteral("Remove old team id(%0) image: %1").arg(team->m_id).arg(file_path + QLatin1String("/image.png"));
			dir.remove(file_path + QLatin1String("/image.png"));
		}

		dir.mkpath(file_path);
		file_path += QLatin1String("/image.png");
		QFile save(file_path);
		if(save.open(QIODevice::WriteOnly))
		{
			save.write( replyData );
			save.close();
			team->m_image_path = file_path;
			qInfo() << QStringLiteral("Team \"%0\" id(%1) image saved: %2").arg(team->m_display_name).arg(team->m_id).arg(file_path);
		}
		else
			qWarning() << QStringLiteral("Cant save team id(%1) image: %0").arg(file_path).arg(team->m_id);
	}
	if(!team->m_image_path.isEmpty())
	{
		QVectorInt roles;
		roles << TeamsModel::TeamIcon;
		emit teamChanged(team, roles);
	}
	else
		qWarning() << QStringLiteral("Team id(%0) image not saved!").arg(team->m_id);
}

void MattermostQt::reply_get_post(QNetworkReply *reply)
{
	int server_index = reply->property(P_SERVER_INDEX).toInt();
	//	int team_index = reply->property(P_TEAM_INDEX).toInt();
	//	int channel_index = reply->property(P_CHANNEL_INDEX).toInt();
	//	int channel_type =reply-> property(P_CHANNEL_TYPE).toInt();

	//	ChannelPtr channel = channelAt(server_index,team_index,channel_type,channel_index);
	if(server_index <0 || server_index >=m_server.size())
	{
		qWarning() << "Wrong server index";
		return;
	}

	MessagePtr message = reply-> property(P_MESSAGE_PTR).value<MessagePtr>();

	if(message.isNull())
	{
		qWarning() << "Cant read Message pointer";
		return;
	}

	QJsonDocument json = QJsonDocument::fromJson(reply->readAll());

	ServerPtr sc = m_server[server_index];
	MessagePtr mc(new MessageContainer(json.object(), sc->m_user_id));
	//for first, find user name
	//	mc->m_user_id
	for(int k = 0; k < sc->m_user.size(); k++)
	{
		if( mc->m_user_id.compare(sc->m_user[k]->m_id) == 0)
		{
			message->m_root_user_name = sc->m_user[k]->m_username;
			message->m_root_user_index = k;
			break;
		}
	}
	if(message->m_root_user_name.isEmpty())
		message->m_root_user_name = "unkown";
	message->m_root_message = SettingsContainer::getInstance()->strToSingleLine( mc->m_message ); // not formatted text
	if(message->m_root_message.isEmpty())
	{// here we need generate message from files list
		// get file info, and write file name
		for( int i = 0 ; i < mc->m_file_ids.size(); i ++ )
		{
			message->m_root_message += tr("File: id(%0); ").arg(mc->m_file_ids[i]);
		}
	}
	//now send signal - message is changed
	QList<MessagePtr> mlist;
	mlist.push_back(message);
	emit messageUpdated(mlist);
	//TODO if user not exists, need request for user data (!)
}

void MattermostQt::reply_get_posts(QNetworkReply *reply)
{
#ifdef MQT_TRACE
	qInfo() << "MattermostQt::reply_get_posts()";
#endif
	int server_index = reply->property(P_SERVER_INDEX).toInt();
	int team_index = reply->property(P_TEAM_INDEX).toInt();
	int channel_index = reply->property(P_CHANNEL_INDEX).toInt();
	int channel_type =reply-> property(P_CHANNEL_TYPE).toInt();

	ChannelPtr channel = channelAt(server_index,team_index,channel_type,channel_index);
	if(!channel)
	{
		qCritical() << QStringLiteral("Chanel not found! ").arg(server_index).arg(team_index).arg(channel_type).arg(channel_index);
		return;
	}
	//	if(channel->m_type == ChannelDirect)
	//		team_index = -1;
	QList<MessagePtr> m_replies;

	ServerPtr sc = m_server[server_index];
	QJsonDocument json = QJsonDocument::fromJson(reply->readAll());
	QJsonArray order = json.object()["order"].toArray();
	channel->m_message.reserve(order.size());
	QJsonObject posts = json.object()["posts"].toObject();
	QJsonObject::iterator it = posts.begin(),
	        end = posts.end();
	bool new_messages = false;
	int its_all = order.size();
	QList<MessagePtr> answers;
	for(; it != end; it++ )
	{
		MessagePtr message(new MessageContainer(it.value().toObject(), sc->m_user_id));
		message->m_server_index = server_index;
		if(channel_type != ChannelDirect)
			message->m_team_index = team_index;
		else
			message->m_team_index = -1;
		message->m_channel_id = channel->m_id;
		message->m_channel_type = channel->m_type;
		message->m_channel_index = channel->m_self_index;
		message->m_self_index = channel->m_message.size();
		channel->m_message.append(message);
		if(message->m_type == MessageOwner::MessageTypeCount)
		{
			if(message->m_user_id.compare(sc->m_user_id) == 0 )
				message->m_type = MessageMine;
			else
				message->m_type = MessageOther;
		}

		// get user_index for post
		prepare_user_index(sc->m_self_index, message, team_index);
		// check if message is answer
		if( !message->m_root_id.isEmpty() )
		{
			answers.append(message);
		}

		new_messages = true;
	}

	if(new_messages)
	{
		QVector<MessagePtr> &messages = channel->m_message;
		// sorting with right order
		for(int i = 0, j2 = messages.size() - 1; i < order.size(); i++, j2 = messages.size() - 1 - i)
		{
			QString id = order[i].toString();
			for(int j = messages.size() - i - 1; j >= 0  ; j--)
			{
				if( messages[j]->m_id.compare(id) == 0 )
				{
					MessagePtr temp;
					temp = messages[j];
					messages[j] = messages[j2];
					messages[j2] = temp;
					messages[j2]->m_self_index = j2;
					messages[j]->m_self_index = j;
					for(QList<MessagePtr>::iterator it = answers.begin(); it != answers.end(); it++)
					{
						MessagePtr ans = *it;
						if( messages[j]->m_id == ans->m_root_id )
						{
							ans->m_root_ptr = messages[j];
							messages[j]->m_thread_messages.append(ans);
							it = answers.erase(it);
							ans->updateRootMessage(this);
							//							break;
						}
						else if( messages[j2]->m_id == ans->m_root_id )
						{
							ans->m_root_ptr = messages[j2];
							messages[j2]->m_thread_messages.append(ans);
							it = answers.erase(it);
							ans->updateRootMessage(this);
							//							break;
						}
						if( it == answers.end() )
							break;
					}
#ifdef PRELOAD_FILE_INFOS
					// TODO remove get_file_info, becuse it need only when we start view message
					for(int k = 0; k < temp->m_file_ids.size(); k++ )
					{
						get_file_info(
						            server_index,
						            team_index,
						            channel_type,
						            channel_index,
						            temp->m_self_index,
						            temp->m_file_ids[k]);
					}
#endif
					break;
				}
			}
		}
		// here we request root post by id, if there not already loaded
		for(QList<MessagePtr>::iterator it = answers.begin(), end = answers.end(); it != end; it++)
		{
			MessagePtr message = *it;
			get_post(server_index, message->m_root_id, message);
		}
		emit messagesAdded(channel);
		if(its_all < 60 )
			emit messagesIsEnd(channel);
	}
#ifdef MQT_TRACE
	qInfo() << "MattermostQt::reply_get_posts() end function";
#endif
}

void MattermostQt::reply_get_posts_before(QNetworkReply *reply)
{
	int server_index = reply->property(P_SERVER_INDEX).toInt();
	int team_index = reply->property(P_TEAM_INDEX).toInt();
	int channel_index = reply->property(P_CHANNEL_INDEX).toInt();
	int channel_type = reply-> property(P_CHANNEL_TYPE).toInt();
	//	int before_message_index = reply-> property(P_MESSAGE_INDEX).toInt();

	ChannelPtr channel = channelAt(server_index,team_index,channel_type,channel_index);
	if(!channel)
	{
		qWarning() << QStringLiteral("Chanel not found!");
		return;
	}
	ServerPtr sc = m_server[server_index];

	QVector<MessagePtr> messages;
	QJsonDocument json = QJsonDocument::fromJson(reply->readAll());

	QJsonArray order = json.object()["order"].toArray();
	QJsonObject posts = json.object()["posts"].toObject();
	QJsonObject::iterator it = posts.begin(),
	        end = posts.end();
	messages.reserve(order.size() + channel->m_message.size());
	bool new_messages = false;
	int its_all = order.size();
	// list of answer messages
	QList<MessagePtr> answers;
	for(; it != end; it++ )
	{
		MessagePtr message(new MessageContainer(it.value().toObject(), sc->m_user_id));
		message->m_server_index = server_index;
		if(channel_type != ChannelDirect)
			message->m_team_index = team_index;
		else
			message->m_team_index = -1;
		message->m_channel_id = channel->m_id;
		message->m_channel_type = channel->m_type;
		message->m_channel_index = channel->m_self_index;
		messages.append(message);
		if(message->m_type == MessageOwner::MessageTypeCount)
		{
			if(message->m_user_id.compare(sc->m_user_id) == 0 )
				message->m_type = MessageMine;
			else
				message->m_type = MessageOther;
		}
		prepare_user_index(server_index,message);
		// check if message is answer
		if( !message->m_root_id.isEmpty() )
		{
			answers.append(message);
		}
		new_messages = true;
	}
	if(!new_messages)
		return;

	// sorting with right order
	for(int i = 0, j2 = messages.size() - 1; i < order.size(); i++, j2 = messages.size() - 1 - i)
	{
		QString id = order[i].toString();
		for(int j = messages.size() - i - 1; j >= 0  ; j--)
		{
			if( messages[j]->m_id.compare(id) == 0 )
			{
				MessagePtr temp;
				temp = messages[j];
				messages[j] = messages[j2];
				messages[j2] = temp;
				messages[j2]->m_self_index = j2;
				messages[j]->m_self_index = j;

				for(QList<MessagePtr>::iterator it = answers.begin(); it != answers.end(); it++)
				{
					MessagePtr ans = *it;
					if( messages[j]->m_id == ans->m_root_id )
					{
						ans->m_root_ptr = messages[j];
						messages[j]->m_thread_messages.append(ans);
						ans->updateRootMessage(this);
						it = answers.erase(it);
						//						break;
					}
					else if( messages[j2]->m_id == ans->m_root_id )
					{
						ans->m_root_ptr = messages[j2];
						messages[j2]->m_thread_messages.append(ans);
						ans->updateRootMessage(this);
						it = answers.erase(it);
						//						break;
					}
					if( it == answers.end() )
						break;
				}
				break;
			}
		}
	}
	// here we request root post by id, if there not already loaded
	for(QList<MessagePtr>::iterator it = answers.begin(), end = answers.end(); it != end; it++)
	{
		MessagePtr message = *it;
		get_post(message->m_server_index, message->m_root_id, *it);
	}
	int size = messages.size();
	// now add new messages to front, and change all indexes after new
	for(int i = 0; i < channel->m_message.size(); i++)
		channel->m_message[i]->m_self_index += size;
	messages.append(channel->m_message);
	channel->m_message.swap(messages);
	// get files info from new messages
#ifdef PRELOAD_FILE_INFOS
	for(int i = 0; i < size; i++)
	{
		MessagePtr temp = channel->m_message[i];
		for(int k = 0; k < temp->m_file_ids.size() && false; k++ )
		{
			get_file_info(
			            server_index,
			            team_index,    //temp->m_team_index,
			            channel_type,  //temp->m_channel_type,
			            channel_index, //temp->m_channel_index,
			            temp->m_self_index,
			            temp->m_file_ids[k]);
		}
	}
#endif
	emit messagesAddedBefore(channel, size);

}

void MattermostQt::reply_get_public_channels(QNetworkReply *reply)
{
	ASSERT_REPLY(reply_get_public_channels)
	        int server_index = reply->property(P_SERVER_INDEX).toInt();
	if( server_index < 0 || server_index >= m_server.size() )
	{
		qWarning() << "Wrong server index";
		return;
	}
	ServerPtr sc = m_server[server_index];
	TeamPtr tc;

	QJsonDocument json = QJsonDocument::fromJson(reply->readAll());
	//qDebug() << json;
	if(json.isArray())
	{
		QJsonArray array = json.array();
		for(int i = 0; i < array.size(); i++ )
		{
			if(array.at(i).isObject())
			{
				QJsonObject object = array.at(i).toObject();
				ChannelPtr ct( new ChannelContainer(object) );

				// TODO not much safe, need test all parameters
				ct->m_server_index = server_index;
				QString channel_type;

				switch( ct->m_type )
				{
				// open channels
				case MattermostQt::ChannelPublic:
					ct->m_team_index = reply->property(P_TEAM_INDEX).toInt();
					tc.reset();
					tc = sc->m_teams[ct->m_team_index];
					ct->m_self_index = tc->m_public_channels.size();
					tc->m_public_channels.append(ct);
					channel_type = QStringLiteral("public");
					emit channelAdded(ct);
					break;
					// private channels
				case MattermostQt::ChannelPrivate:
					ct->m_team_index = reply->property(P_TEAM_INDEX).toInt();
					tc.reset();
					tc = sc->m_teams[ct->m_team_index];
					ct->m_self_index = tc->m_private_channels.size();
					tc->m_private_channels.append(ct);
					channel_type = QStringLiteral("private");
					emit channelAdded(ct);
					break;
					// direct channel
				case MattermostQt::ChannelDirect:
				{
					bool channel_exists = false;
					// search if channels already added
					// TODO need refactoring ( because direct channels must be added once
					for(int c = 0; c < m_server[ct->m_server_index]->m_direct_channels.size(); c++)
					{
						if(m_server[ct->m_server_index]->m_direct_channels[c]->m_id.compare(ct->m_id) == 0 )
						{
							int si = ct->m_server_index;
							ct.reset();
							ct = m_server[si]->m_direct_channels[c];
							channel_exists = true;
							break;
						}
					}
					if(channel_exists)
					{
						emit channelAdded(ct);
						break;
					}
					ct->m_team_index = -1;
					ct->m_self_index = m_server[ct->m_server_index]->m_direct_channels.size();
					m_server[ct->m_server_index]->m_direct_channels.append(ct);
					channel_type = QStringLiteral("dircet");
					emit channelAdded(ct);
					prepare_direct_channel(ct->m_server_index, ct->m_self_index);
				}
					break;
				default:
					break;
				}
				if(!channel_type.isEmpty())
					qDebug() << QStringLiteral("Add %0 channel with \"%1\" : %2").arg(channel_type).arg(ct->m_display_name).arg(ct->m_id);
			}
		}
	}
	post_users_status(server_index);
}

void MattermostQt::reply_get_channel(QNetworkReply *reply)
{
	ASSERT_REPLY(reply_get_channel)
	        QJsonDocument json = QJsonDocument::fromJson(reply->readAll());
	QJsonObject object = json.object();
	ChannelPtr channel(new ChannelContainer(object));
	if(!channel)
	{
		qWarning() << json;
		return;
	}

	int server_index = reply->property(P_SERVER_INDEX).toInt();
	if(server_index<0 || server_index>= m_server.size())
	{
		qWarning() << "Wrong server index";
		return;
	}
	channel->m_server_index = server_index;

	ServerPtr sc = m_server[server_index];
	// if we here, it seems we dont have that channel in any list
	if( channel->m_type == ChannelDirect )
	{
		channel->m_self_index = sc->m_direct_channels.size();
		channel->m_team_index = -1;
		sc->m_direct_channels.append(channel);
		emit channelAdded(channel);
		get_posts(server_index,-1,channel->m_type, channel->m_self_index);
		prepare_direct_channel(server_index,channel->m_self_index);
	}
	else
	{
		TeamPtr team;
		for(int i = 0; i < sc->m_teams.size(); i++ )
		{
			if( sc->m_teams[i]->m_id  == channel->m_team_id )
			{
				team = sc->m_teams[i];
				break;
			}
		}
		if(!team)
		{
			qWarning() << "Team not found";
			// its may be if team is new
			// TODO get_team
			return;
		}
		if(channel->m_type == ChannelPublic)
		{
			channel->m_self_index = team->m_public_channels.size();
			team->m_public_channels.append(channel);
		}
		else
		{
			channel->m_self_index = team->m_public_channels.size();
			team->m_public_channels.append(channel);
		}
		channel->m_team_index = team->m_self_index;
		channelAdded(channel);
		get_posts(server_index,channel->m_team_index,channel->m_type, channel->m_self_index);
	}
	emit updateChannelInfo(channel->m_id, channel->m_team_index, channel->m_self_index);
}

void MattermostQt::reply_post_channel_view(QNetworkReply *reply)
{
	ASSERT_REPLY(reply_post_channel_view)
	        ChannelPtr channel = reply->property(P_CHANNEL_PTR).value<ChannelPtr>();
	if(!channel)
		return;
	ServerPtr pc = m_server[channel->m_server_index];
	QJsonDocument json = QJsonDocument::fromJson(reply->readAll());
	QJsonObject object = json.object();
	if( object["status"].toString() != QString("success") )
	{
		qWarning() << json;
	}
	if( channel->m_type != ChannelDirect )
	{
		TeamPtr team = teamAt(channel->m_server_index, channel->m_team_index);
		get_teams_unread(pc);
		//		team->m_unread_messages -= channel->m_msg_unread;
		//		team->m_unread_mentions -= channel->m_mention_count;
		//		emit teamChanged(team, QVectorInt() << TeamsModel::RoleUnreadMentionCount << TeamsModel::RoleUnreadMessageCount );
	}
	channel->m_mention_count = 0;
	channel->m_msg_unread = 0;
	emit updateChannel(channel, QVectorInt() << ChannelsModel::MentionCount << ChannelsModel::MessageUnread );
	emit messageUnreadChanged();
}

void MattermostQt::reply_get_channel_unread(QNetworkReply *reply)
{
	ASSERT_REPLY(reply_get_channel_unread)
	        ChannelPtr channel = reply->property(P_CHANNEL_PTR).value<ChannelPtr>();
	if(!channel)
		return;
	ServerPtr pc = m_server[channel->m_server_index];
	QJsonDocument json = QJsonDocument::fromJson(reply->readAll());
	QJsonObject object = json.object();
	if( object["channel_id"].toString() != channel->m_id )
	{
		qCritical() << json;
	}
	channel->m_msg_unread    = object["msg_count"].toInt();
	channel->m_mention_count = object["mention_count"].toInt();
	emit updateChannel( channel, QVectorInt() << ChannelsModel::MessageUnread << ChannelsModel::MentionCount );
	emit messageUnreadChanged();
}

void MattermostQt::reply_get_user_info(QNetworkReply *reply)
{
	ASSERT_REPLY(reply_get_user_info)
	        int server_index = reply->property(P_SERVER_INDEX).toInt();
	int team_index = reply->property(P_TEAM_INDEX).toInt();
	bool direct_channel = reply->property(P_DIRECT_CHANNEL).toBool();
	QString user_id = reply->property(P_USER_ID).toString();

	if( server_index < 0 || server_index >= m_server.size() )
	{
		qWarning() << "Error! Wrong server index";
		return;
	}
	ServerPtr sc = m_server[server_index];

	// first delete user from request list
	for(int u = 0; u < sc->m_requested_users.size(); u++ )
	{
		if( sc->m_requested_users[u] == user_id )
		{
			sc->m_requested_users.remove(u);
			break;
		}
	}

	QJsonDocument json = QJsonDocument::fromJson(reply->readAll());

	// qDebug() << json; // protected data

	if( !json.isObject() )
	{
		qWarning() << "Cant parse Json";
		return;
	}

	UserPtr user( new UserContainer(json.object()) );
	// first check if user is current account
	if( sc->m_user_id == user->m_id && team_index == GET_USER_INFO_current_user ) {
		sc->m_current_user = user;
	}
	//	for(int)
	bool user_exists = false;
	// maby need check if user laready exists in list
	for(int i = 0; i < sc->m_user.size(); i ++ )
	{
		if( user->m_id.compare(sc->m_user[i]->m_id) == 0 )
		{
			user.reset();
			user = sc->m_user[i];
			user_exists = true;
			break;
		}
	}
	if(!user_exists)
	{
		user->m_self_index = sc->m_user.size();
		sc->m_user.append(user);

		QString path = sc->m_cache_path +
		        QString("/users/") +
		        user->m_id +
		        QString("/image.png");
		// check if user has image
		QFile user_image(path);
		bool user_image_exists = user_image.exists();

		QList<MessagePtr>::iterator it = sc->m_nouser_messages.begin();
		for( ;it != sc->m_nouser_messages.end(); )
		{
			MessagePtr m = *it;
			if(m->m_user_id == user->m_id)
			{
				if(m->m_user_index == -1)
				{
					m->m_user_index = user->m_self_index;
					emit updateMessage(m,MessagesModel::SenderUserName);
				}
				// check if user has image
				if( user_image_exists ) {
					emit updateMessage(m,MessagesModel::SenderImagePath);
					it = sc->m_nouser_messages.erase(it);
					user->m_image_path = path;
					continue;
				}
			}
			it++;
		}

		//		if( !user_image_exists )
		get_user_image(server_index,user->m_self_index);
	}

	bool self_chat = sc->m_user_id == user->m_id;

	if(direct_channel && team_index == -1)
	{
		for(int i = 0; i < sc->m_direct_channels.size(); i++)
		{
			ChannelPtr channel = sc->m_direct_channels[i];

			QString ch_name = channel->m_name;
			ch_name = ch_name.replace(QRegExp(QString("(%0|__)").arg(sc->m_user_id)),"");
			if(self_chat && ch_name.isEmpty())
			{
				channel->m_display_name = user->m_username
				        + QLatin1String(" ") // for right translation, if someone forgot about space before '(you)'
				        + QObject::trUtf8("(you)");
				channel->m_dc_user_index = user->m_self_index;
				QVectorInt roles;
				roles << ChannelsModel::DisplayName << ChannelsModel::AvatarPath;
				emit updateChannel(channel, roles);
				qDebug() << QString("direct_channel \"%0\" id: '%1'").arg(channel->m_display_name).arg(channel->m_id) ;
				break;
			}
			else if( ch_name == user->m_id  )
			{
				QString displayname = user->m_username;
				if( SettingsContainer::getInstance()->userNameDisplay() == SettingsContainer::UserNameFull )
				{
					if( !user->m_first_name.isEmpty() || !user->m_last_name.isEmpty() )
					{
						displayname = QStringLiteral("%0 %1").arg(user->m_first_name).arg(user->m_last_name);
					}
				}
				else if( SettingsContainer::getInstance()->userNameDisplay() == SettingsContainer::UserNameNickname ) {
					if( !user->m_nickname.isEmpty() )
						displayname = user->m_nickname;
				}

				QVectorInt roles;
				roles << ChannelsModel::DisplayName
				      << ChannelsModel::AvatarPath
				      << ChannelsModel::RoleUserFullName
				      << ChannelsModel::RoleUserNickname;

				channel->m_display_name = displayname;
				channel->m_dc_user_index = user->m_self_index;

				emit updateChannel(channel, roles);
//				qDebug() << QString("direct_channel \"%0\" id: '%1'").arg(channel->m_display_name).arg(channel->m_id) ;
				break;
			}
		}
		if(user->m_image_path.isEmpty() && user_exists)
			get_user_image(server_index,user->m_self_index);
	}
}

void MattermostQt::error_get_user_info(QNetworkReply *reply)
{
	int server_index = reply->property(P_SERVER_INDEX).toInt();
	QString user_id = reply->property(P_USER_ID).toString();

	if( server_index < 0 || server_index >= m_server.size() )
	{
		qCritical() << "error_get_user_info():: Error! Wrong server index";
		return;
	}
	ServerPtr sc = m_server[server_index];

	// first delete user from request list
	for(int u = 0; u < sc->m_requested_users.size(); u++ )
	{
		if( sc->m_requested_users[u] == user_id )
		{
			sc->m_requested_users.remove(u);
			break;
		}
	}
}

void MattermostQt::reply_post_users_status(QNetworkReply *reply)
{
	int server_index = reply->property(P_SERVER_INDEX).toInt();
	if(server_index < 0 || server_index >= m_server.size() )
	{
		qWarning() << "Wrong server index ";
		return;
	}
	ServerPtr sc = m_server[server_index];
	QByteArray replyData = reply->readAll();
	QJsonDocument json = QJsonDocument::fromJson(replyData);
	QVector<UserPtr> users;
	users.reserve(sc->m_user.size());
	if( !json.isEmpty())
	{
		if(json.isArray() )
		{
			QJsonArray array = json.array();
			for( int i = 0; i < array.size(); i++ )
			{
				QJsonObject userObject = array[i].toObject();
				QString id = userObject["user_id"].toString();
				UserPtr current = id2user(sc,id);
				if(current)
				{
					QString s = userObject["status"].toString();
					UserStatus status = str2status(s);
					if( current->m_status != status )
					{
						current->m_status = status;
						current->m_last_activity_at = (qlonglong)userObject["last_activity_at"].toDouble(0);
						users << current;
					}
				}
			}
		}
		else
			qWarning() << json;
	}
	else if ( replyData.size() > 0 )
		qWarning() << replyData.data();
	if(users.empty()) return;
	// then send signal with updated users
	QVectorInt roles;
	roles << UserLastActivityRole << UserStatusRole;
	emit usersUpdated(users,roles);
}

void MattermostQt::reply_error(QNetworkReply *reply)
{
	QByteArray replyData = reply->readAll();
	QJsonDocument json = QJsonDocument::fromJson(replyData);
	if( !json.isEmpty())
	{
		if(json.isObject() )
		{
			QJsonObject object = json.object();
			QString error_id = object["id"].toString();
			QString message = object["message"].toString();
			qDebug() << object;
			if( error_id.indexOf("api.") == 0 ) {
				error_id = error_id.mid(4);
				if( error_id.compare("user.check_user_password.invalid.app_error") == 0 )
				{
					emit onConnectionError(ConnectionError::WrongPassword, QObject::tr("Login failed because of invalid password"), -1 );
				}
				else if( error_id.compare("context.session_expired.app_error") == 0 )
				{
					int server_index  = reply->property(P_SERVER_INDEX).toInt();
					emit onConnectionError(ConnectionError::SessionExpired, message, server_index);
					ReplyType type = reply->property(P_REPLY_TYPE).value<ReplyType>();
					// output request authorisation header
					qWarning() << QStringLiteral("Reply type: %0").arg( QVariant::fromValue<ReplyType>(type).toString() );
//					QList<QByteArray> headers =  reply->rawHeaderList();
					QList<QNetworkReply::RawHeaderPair> headers = reply->rawHeaderPairs();
					QString authheader("Authorization");
					QString message;
					for(auto header : headers) {
						if( authheader.indexOf(header.first)!= -1 )
						{
							QString token = header.second;
							message += QString("  Header token: ") + token;
							message += QString("\n  Stored token: ") + m_server[server_index]->m_bearer + QStringLiteral("\n");
						}
						else
						{
							message += QStringLiteral("  Header: \"") + QString(header.first) + QStringLiteral("\" : ") + header.second + QStringLiteral("\n");
						}
					}
					qCritical() << message;
				}
				else if( error_id.compare("user.login.invalid_credentials_email_username") == 0 )
				{
					int server_index  = reply->property(P_SERVER_INDEX).toInt();
					emit onConnectionError(ConnectionError::WrongPasswordOrEmail, message, server_index);
				}
				else if( error_id.indexOf("team.") == 0 ) {
					error_id = error_id.mid(5);
					if( error_id == QStringLiteral("get_team_icon.read_file.app_error") ){
						// team has no icon
						int si = reply->property(P_SERVER_INDEX).toInt();
						int ti = reply->property(P_TEAM_INDEX).toInt();
						TeamPtr team = teamAt(si,ti);
						if(team) {
							team->m_image_path = "";
							qWarning() << QStringLiteral("Team id(%0) has no icon.").arg(team->m_id);
							ServerPtr server = m_server[si];
							QString file_path = server->m_cache_path
							        + QLatin1String("/teams/")
							        + team->m_id;
							QDir dir;
							if( QFile::exists(file_path + QLatin1String("/image.png")) )
							{
								qWarning() << QStringLiteral("Remove old team id(%0) image: %1").arg(team->m_id).arg(file_path + QLatin1String("/image.png"));
								dir.remove(file_path + QLatin1String("/image.png"));
							}
						}
						else
							qWarning() << QStringLiteral("Team has no icon.");
					}
				}
				else if( error_id.indexOf("file.") == 0) {
					error_id = error_id.mid(5);
					if( error_id.indexOf("get_file_preview.no_preview") == 0 ) {
						// file has no preview, it may by SVG =)
						// TODO do somethig with this ERROR!
					}
				}
			}
			else {
				int server_index  = reply->property(P_SERVER_INDEX).toInt();
				emit onConnectionError(ConnectionError::UnknownError, QStringLiteral("(%0): %1").arg(error_id).arg(message), server_index);
				qWarning() << QStringLiteral("Unparsed error json: %0").arg(QVariant(json).toString());
			}
		}
#ifdef _DEBUG
		qWarning() << json;
#endif

	}
	else if ( replyData.size() > 0 )
	{
		QString message = replyData.data();
		emit onConnectionError(ConnectionError::UnknownError, message, -1);
		qWarning() << replyData.data();
	}
}

void MattermostQt::reply_get_file_thumbnail(QNetworkReply *reply)
{
	// we think all indexes right
	int server_index = reply->property(P_SERVER_INDEX).toInt();
	int file_sc_index = reply->property(P_FILE_SC_INDEX).toInt();
	FilePtr file = m_server[server_index]->m_file[file_sc_index];
	int team_index = file->m_team_index;
	int channel_type = file->m_channel_type;
	int channel_index = file->m_channel_index;
	int message_index = file->m_message_index;

	ServerPtr sc = m_server[server_index];
	ChannelPtr channel;
	if(channel_type != ChannelDirect)
	{
		TeamPtr tc = sc->m_teams[team_index];
		if( channel_type == ChannelType::ChannelPublic )
			channel = tc->m_public_channels[channel_index];
		else// if( channel_type == ChannelType::ChannelPublic )
			channel = tc->m_private_channels[channel_index];
	}
	else
		channel = sc->m_direct_channels[channel_index];

	MessagePtr mc = channel->m_message[message_index];

	QByteArray replyData = reply->readAll();
	{
		QString file_path = sc->m_cache_path
		        + QLatin1String("/files/")
		        + file->m_id;
		QDir dir;
		if( !QFile::exists(file_path + QLatin1String("/thumb.jpeg")) )
		{
			dir.mkpath(file_path);
			file_path += QLatin1String("/thumb.jpeg");
			QFile save(file_path);
			if(save.open(QIODevice::WriteOnly | QIODevice::Truncate))
			{
				save.write( replyData );
				save.close();
				file->m_thumb_path = file_path;
			}
			else
				qDebug() << file_path;
		}
		else
			file->m_thumb_path = file_path + QLatin1String("/thumb.jpeg");
	}
	if(!file->m_thumb_path.isEmpty())
	{
		//		QList<MessagePtr> messages;
		//		messages << mc;
		//		emit messageUpdated(messages);
		QVectorInt file_update_roles;
		file_update_roles << AttachedFilesModel::FileThumbnailPath;
		emit attachedFilesChanged(mc, QVector<QString>(),file_update_roles);
	}
	return;
}

void MattermostQt::reply_get_file_preview(QNetworkReply *reply)
{
	// we think all indexes right
	int server_index = reply->property(P_SERVER_INDEX).toInt();
	int file_sc_index = reply->property(P_FILE_SC_INDEX).toInt();
	FilePtr file = m_server[server_index]->m_file[file_sc_index];
	int team_index = file->m_team_index;
	int channel_type = file->m_channel_type;
	int channel_index = file->m_channel_index;
	int message_index = file->m_message_index;

	ChannelPtr channel = channelAt(server_index,team_index,channel_type,channel_index);
	if(!channel)
	{
		qWarning() << "Cant found chanel";
		return;
	}
	ServerPtr sc = m_server[server_index];
	MessagePtr mc = channel->m_message[message_index];

	QByteArray replyData = reply->readAll();
	{
		QString file_path = sc->m_cache_path
		        + QLatin1String("/files/")
		        + file->m_id;
		QDir dir;
		if( !QFile::exists(file_path + QLatin1String("/preview.jpeg")) )
		{
			dir.mkpath(file_path);
			file_path += QLatin1String("/preview.jpeg");
			QFile save(file_path);
			if(save.open(QIODevice::WriteOnly | QIODevice::Truncate))
			{
				save.write( replyData );
				save.close();
				file->m_preview_path = file_path;
			}
			else
				qDebug() << file_path;
		}
		else
			file->m_preview_path = file_path + QLatin1String("/preview.jpeg");
	}
	if(!file->m_preview_path.isEmpty())
	{
		//		QList<MessagePtr> messages;
		//		messages << mc;
		//		emit messageUpdated(messages);
		QVectorInt file_update_roles;
		file_update_roles << AttachedFilesModel::FilePreviewPath << MessagesModel::FilePreview;
		emit attachedFilesChanged(mc, QVector<QString>(), file_update_roles);
	}
	return;
}

void MattermostQt::reply_get_file_info(QNetworkReply *reply)
{
	// we think all indexes right
	int server_index  = reply->property(P_SERVER_INDEX).toInt();
	int team_index    = reply->property(P_TEAM_INDEX).toInt();
	int channel_type  = reply->property(P_CHANNEL_TYPE).toInt();
	int channel_index = reply->property(P_CHANNEL_INDEX).toInt();
	int message_index = reply->property(P_MESSAGE_INDEX).toInt();
	int file_index    = reply->property(P_FILE_INDEX).toInt();

	MessagePtr mc = messageAt(server_index,team_index,channel_type,channel_index,message_index);
	if(!mc)
	{
		qWarning() << "Cant found message ";
		return;
	}
	ServerPtr sc = m_server[server_index];

	QByteArray replyData = reply->readAll();
	QJsonDocument json = QJsonDocument::fromJson(replyData);
	//#ifdef NO_LOCK
	//	FilePtr file(new FileContainer(json.object()));
	FilePtr file = mc->fileAt(file_index);

	if(!file){
		qCritical() << "Something went wrong! File index is empty in reply_get_file_info()!";
		QString file_id   = reply->property(P_FILE_ID).toString();
		for(int fi = 0 ; fi < mc->m_file.size(); fi++ )
		{
			if( file_id == mc->m_file[fi]->m_id )
			{
				file = mc->m_file[fi];
				break;
			}
		}
		if( !file )
		{
			qCritical() << QStringLiteral("File with id(%0) not found in message with id(%1)").arg(file_id).arg(mc->m_id);
			return;
		}
	}
	file->parse_from_json(json.object());

	//	file->m_self_index = mc->m_file.size();
	//	mc->m_file.push_back(file);

	//QList<MessagePtr> messages;
	QVectorInt file_update_roles;

	file_update_roles << AttachedFilesModel::FileType;
	file_update_roles << AttachedFilesModel::FileName;
	file_update_roles << AttachedFilesModel::FileSize;
	file_update_roles << AttachedFilesModel::FileMimeType;
	file_update_roles << MessagesModel::RoleFileInfo;
	//	file_update_roles << AttachedFilesModel::FileCount;
	//messages << mc;
	bool isUpdateMessage = false;

	file->m_self_sc_index = m_server[server_index]->m_file.size();
	m_server[server_index]->m_file.append(file);
	file->m_server_index = server_index;
	file->m_team_index = team_index;
	file->m_channel_index = channel_index;
	file->m_channel_type = channel_type;
	file->m_message_index = message_index;

	bool download_current_file = false;

	if(file->m_file_type == FileImage || file->m_file_type == FileAnimatedImage )
	{
		file_update_roles << AttachedFilesModel::FileImageSize;
		//		QString file_thumb_path = sc->m_cache_path + QString("/files/%0/thumb.jpeg").arg(file->m_id);
		if( file->m_thumb_path.isEmpty() )
			file->m_thumb_path = sc->m_cache_path + QString("/files/%0/thumb.jpeg").arg(file->m_id);
		if( !QFile::exists(file->m_thumb_path) ) {
			file->m_thumb_path = QString("");
			get_file_thumbnail(server_index,file->m_self_sc_index);
		}
		else {
			file_update_roles << AttachedFilesModel::FileThumbnailPath;
			isUpdateMessage = true;
		}

		if( m_settings && file->m_file_size <= m_settings->autoDownloadImageSize())
			download_current_file = true;

		if(file->m_has_preview_image && !download_current_file ) {
			if( file->m_preview_path.isEmpty() )
				file->m_preview_path = sc->m_cache_path + QString("/files/%0/preview.jpeg").arg(file->m_id);
			if( !QFile::exists(file->m_preview_path) ) {
				file->m_preview_path.clear();
				get_file_preview(server_index,file->m_self_sc_index);
			}
			else {
				file_update_roles << AttachedFilesModel::FilePreviewPath;
				isUpdateMessage = true;
			}
		}
	}// file image
	// TODO - download files to cahce? while user not request "save to gallery" or "download"
	//	QString file_path = m_documents_path + QDir::separator() + file->filename();
	if(file->m_file_path.isEmpty())
		file->m_file_path = m_documents_path + QDir::separator() + file->filename();
	if( !QFile::exists(file->m_file_path) ) {
		file->m_file_path.clear();
		if(download_current_file)
			get_file(file->m_server_index, file->m_team_index,
			         file->m_channel_type, file->m_channel_index,
			         file->m_message_index, file->m_self_index);
	}
	else {
		file->m_file_status = FileStatus::FileDownloaded;
		file_update_roles << AttachedFilesModel::FilePath;
		isUpdateMessage = true;
	}
	//	if(isUpdateMessage)
	//		emit updateMessage(mc, (int)MessagesModel::FilesCount);
	emit attachedFilesChanged(mc, QVector<QString>(), file_update_roles);
	return;
}

void MattermostQt::failed_get_file_info(QNetworkReply *reply)
{
	int server_index  = reply->property(P_SERVER_INDEX).toInt();
	int team_index    = reply->property(P_TEAM_INDEX).toInt();
	int channel_type  = reply->property(P_CHANNEL_TYPE).toInt();
	int channel_index = reply->property(P_CHANNEL_INDEX).toInt();
	int message_index = reply->property(P_MESSAGE_INDEX).toInt();
	int file_index    = reply->property(P_FILE_INDEX).toInt();
	QString file_id   = reply->property(P_FILE_ID).toString();

	//	if( reply->error() == QNetworkReply::TimeoutError )
	//	{// send second request for this file
	//	}

	MessagePtr mc = messageAt(server_index,team_index,channel_type,channel_index,message_index);
	if(!mc)
	{
		qCritical() << "Something went wrong! Cant found MessagePtr in failed_get_file_info()!";
		return;
	}

	FilePtr file = mc->fileAt(file_index);
	if(!file){
		qCritical() << "Something went wrong! File index is emty in failed_get_file_info()!";
		return;
	}

	file->m_file_status = FileStatus::FileUninitialized;

	QVectorInt file_update_roles;
	file_update_roles << AttachedFilesModel::FileStatus;
	emit attachedFilesChanged(mc, QVector<QString>(), file_update_roles);
}

void MattermostQt::reply_get_file(QNetworkReply *reply)
{
	FilePtr file =  reply->property(P_FILE_PTR).value<FilePtr>();
	//	if(file)
	//		qDebug() << file->m_name;
	if( file->m_server_index < 0 || file->m_server_index >= m_server.size() )
	{
		// TODO log Critical error
		return ;
	}
	ServerPtr server = m_server[file->m_server_index];
	QString dowload_dir;
	if(file->m_file_type == FileImage || file->m_file_type == FileAnimatedImage) {
		dowload_dir = file->filedir(server->m_cache_path);
	} else
		dowload_dir = m_download_path;
	// TODO first, save all data to cache dir, and save it to downloads, and images if user request it
	// download_dir => cache_dir
	QDir dir(dowload_dir);
	if( !dir.exists() )
		dir.mkpath(dowload_dir);
	QString filename = file->m_name;
	int it = 0;
	while(QFile::exists(dowload_dir + QDir::separator() + filename))
	{
		if(it++ > 100)
		{
			filename = file->filename();
		}
		filename = file->m_name + QString("_%0").arg(it);
	}
	dowload_dir +=  QDir::separator() + filename;
	QFile download(dowload_dir);
	if( download.open(QFile::Append) )
	{
		download.write(reply->readAll());
		download.flush();
		download.close();
		file->m_file_path = dowload_dir;
		file->m_file_status = FileStatus::FileDownloaded;
		file->save_json( m_server[file->m_server_index]->m_data_path );
	}
	else
		file->m_file_status = FileStatus::FileRemote;

	{
		ChannelPtr channel = channelAt(
		            file->m_server_index,
		            file->m_team_index,
		            file->m_channel_type,
		            file->m_channel_index
		            );
		if(channel && file->m_message_index >=0 && file->m_message_index < channel->m_message.size())
		{
			MessagePtr message = channel->m_message[file->m_message_index];
			//			QList<MessagePtr> msgs;
			//			msgs << message;
			//			emit messageUpdated( msgs );
			QVectorInt roles;
			roles << AttachedFilesModel::FileStatus << AttachedFilesModel::FilePath;
			emit attachedFilesChanged(message, QVector<QString>(),roles);
		}
	}
	emit fileStatusChanged(file->m_id, file->m_file_status);
	return;
}

void MattermostQt::reply_post_file_upload(QNetworkReply *reply)
{
	int server_index = reply->property(P_SERVER_INDEX).toInt();
	if( server_index < 0 || server_index >= m_server.size() )
		return;
	ServerPtr sc = m_server[server_index];
	ChannelPtr channel = reply->property(P_CHANNEL_PTR).value<ChannelPtr>();
	if(!channel)
		return;
	QString file_path = reply->property(P_FILE_PATH).toString();

	QJsonDocument json = QJsonDocument::fromJson(reply->readAll());
	QJsonObject object = json.object();
	QJsonArray file_infos = object["file_infos"].toArray();
	// we attach just one file at once
	for( int i = 0; i < file_infos.size(); i ++ )
	{
		QJsonObject jfile = file_infos.at(i).toObject();
		FilePtr file( new FileContainer(jfile) );
		file->m_server_index = server_index;
		file->m_self_sc_index = sc->m_file.size();
		file->m_file_path = file_path;
		file->m_file_status = FileDownloaded;
		file->m_channel_type = channel->m_type;
		file->m_team_index = channel->m_team_index;
		file->m_channel_index = channel->m_self_index;

		sc->m_file.append(file);
		// TODO make thumb on device ( or not? )
		//		if( file->m_file_type != FileDocument && file->m_file_type != FileUnknown )
		//			get_file_thumbnail(file->m_server_index, file->m_self_sc_index);
		//		else
		sc->m_unattached_file.append(file);
		file->save_json( m_server[file->m_server_index]->m_data_path );
		emit fileUploaded(file->m_server_index, file->m_self_sc_index);
	}
}

void MattermostQt::reply_get_user_image(QNetworkReply *reply)
{
	int server_index = reply->property(P_SERVER_INDEX).toInt();
	int user_index = reply->property(P_USER_INDEX).toInt();

	if( server_index < 0 || server_index >= m_server.size() )
		return;
	ServerPtr sc = m_server[server_index];
	if( user_index < 0 || user_index >= sc->m_user.size() )
		return;
	UserPtr user = sc->m_user[user_index];

	//	QJsonDocument json = QJsonDocument::fromJson(reply->readAll());

	QByteArray replyData = reply->readAll();
	//qDebug() << replyData;
	{
		if( m_cache_path.isEmpty() || m_cache_path != generateCachePath() )
		{
			qCritical() << QStringLiteral("Wrong cache path. %0").arg(m_cache_path);
			m_cache_path = generateCachePath();
		}
		// strange, check m_cache_path
		if(  sc->m_cache_path.isEmpty() || sc->m_cache_path != m_cache_path + QStringLiteral("/%0_%1").arg(sc->m_self_index).arg(sc->m_user_id) )
		{
			qCritical() << QStringLiteral("Wrong cache path for account: %0").arg(sc->m_cache_path);
			sc->m_cache_path = m_cache_path + QStringLiteral("/%0_%1").arg(sc->m_self_index).arg(sc->m_user_id);
			qDebug() << QStringLiteral("Cache path set to: %0").arg(sc->m_cache_path);
		}
		else {
			qDebug() << QStringLiteral("Account cache path is: %0").arg(sc->m_cache_path);
		}

		if( sc->m_data_path.isEmpty() || sc->m_data_path != m_data_path + QStringLiteral("/%0_%1").arg(sc->m_self_index).arg(sc->m_user_id) )
		{
			qCritical() << QStringLiteral("Wrong data path for account: %0").arg(sc->m_data_path);
			sc->m_data_path = m_data_path + QStringLiteral("/%0_%1").arg(sc->m_self_index).arg(sc->m_user_id);
			qDebug() << QStringLiteral("Data path set to: %0").arg(sc->m_data_path);
		}

		QString file_path = sc->m_cache_path
		        + QLatin1String("/users/")
		        + user->m_id;
		QDir dir;
		if( !QFile::exists(file_path + QLatin1String("/image.png")) )
		{
			dir.mkpath(file_path);
			file_path += QLatin1String("/image.png");
			QFile save(file_path);
			if(save.open(QIODevice::WriteOnly | QIODevice::Truncate))
			{
				save.write( replyData );
				save.close();
				user->m_image_path = file_path;
			}
			else
				qWarning() << "Cant save file " << file_path;
		}
		else
			user->m_image_path = file_path + QLatin1String("/image.png");
	}
	if(!user->m_image_path.isEmpty())
	{
		QList<MessagePtr>::iterator it = sc->m_nouser_messages.begin();
		while( it != sc->m_nouser_messages.end() )
		{
			MessagePtr m = *it;
			if(m->m_user_id == user->m_id)
			{
				m->m_user_index = user->m_self_index;
				emit updateMessage(m,MessagesModel::SenderImagePath);
				it = sc->m_nouser_messages.erase(it);
				continue;
			}
			it++;
		}
		for(int i = 0; i < sc->m_direct_channels.size(); i++ )
		{
			ChannelPtr c = sc->m_direct_channels[i];
			if( c->m_dc_user_index == user->m_self_index )
			{
				QVectorInt roles;
				roles << ChannelsModel::AvatarPath;
				emit updateChannel(c, roles);
				break;
			}
		}
		QVectorInt roles;
		roles << UserImageRole;
		emit userUpdated(user,roles);
	}
	else
		qWarning() << "User image not found!";
	//	qDebug() << reply->readAll();

	//qDebug() << "get user image";
}

void MattermostQt::reply_post_send_message(QNetworkReply *reply)
{
	qDebug() << reply->readAll();
}

void MattermostQt::reply_delete_message(QNetworkReply *reply)
{
	qDebug() << reply->readAll();
}

void MattermostQt::reply_put_message_edit(QNetworkReply *reply)
{
	qDebug() << reply->readAll();
}

void MattermostQt::reply_post_create_reaction(QNetworkReply *reply)
{
	qDebug() << reply->readAll();

	int server_index = reply->property(P_SERVER_INDEX).toInt();
	if( server_index < 0 || server_index >= m_server.size() )
		return;
	ServerPtr sc = m_server[server_index];
}

void MattermostQt::reply_delete_reaction(QNetworkReply *reply)
{
	qDebug() << reply->readAll();
}

void MattermostQt::event_posted(ServerPtr sc, QJsonObject object)
{
	// confedencial data
	//	qDebug() << data;
	QJsonObject data  = object["data"].toObject();
	ChannelType type = ChannelType::ChannelTypeCount;
	QString ch_type = data["channel_type"].toString();
	QJsonValue mentionsv = data["mentions"];
	QJsonObject post = data["post"].toObject();
	if( post.isEmpty() )
	{
		QJsonValue value = data.value("post");
		if( value.type() == QJsonValue::String )
		{
			QString s = value.toString();
			QJsonDocument j = QJsonDocument::fromJson( s.toUtf8() );
			post = j.object();
			if(post.isEmpty())
				return;
		}
		else
			return;
	}
	MessagePtr message( new MessageContainer(post, sc->m_user_id) );

	message->m_server_index = sc->m_self_index;
	if(message->m_type == MessageOwner::MessageTypeCount)
	{
		if(message->m_user_id.compare(sc->m_user_id) == 0 )
			message->m_type = MessageMine;
		else
			message->m_type = MessageOther;
	}
	prepare_user_index(sc->m_self_index,message);

	int team_index = -1;

	if( cmp(ch_type,O) )
		type = ChannelType::ChannelPublic;
	else if( cmp(ch_type,P) )
		type = ChannelType::ChannelPrivate;
	else if( cmp(ch_type,D) )
		type = ChannelType::ChannelDirect;
	message->m_channel_type = type;

	if( type == ChannelType::ChannelDirect )
	{
		QString channel_name = data["channel_name"].toString();
		ChannelPtr channel;
		int channel_index = -1;
		for(int i = 0; i < sc->m_direct_channels.size(); i++ )
		{
			if( scmp(sc->m_direct_channels[i]->m_name,channel_name) )
			{
				channel = sc->m_direct_channels[i];
				channel_index = i;
				break;
			}
		}
		if( channel && channel_index >= 0)
		{
			message->m_channel_type  = channel->m_type;
			message->m_channel_id = channel->m_id;
			message->m_team_index    = -1;
			message->m_channel_index = channel_index;
			message->m_self_index    = channel->m_message.size();
			channel->m_message.append(message);
			channel->m_total_msg_count++;
#ifdef PRELOAD_FILE_INFOS
			for(int k = 0; k < message->m_file_ids.size(); k++ )
			{
				get_file_info(sc->m_self_index,-1,(int)ChannelType::ChannelDirect,channel_index,
				              message->m_self_index, message->m_file_ids[k]);
			}
#endif
			QList<MessagePtr> new_messages;
			new_messages << message;
			emit messageAdded(new_messages); // add messages to model
			// chek if messed sended from another user, then
			if( message->m_type != MessageMine ) {
				// that for notifications
				emit newMessage(message);
				channel->m_msg_unread++;
				//				if( mentionsv.isArray() )
				//					qDebug() << mentionsv;
				emit updateChannel(channel, QVector<int>()<< ChannelsModel::MessageUnread  );
				emit messageUnreadChanged();
			}
		}
	}
	else
	{
		QString team_id = data["team_id"].toString();
		QString channel_id = message->m_channel_id;
		ChannelPtr channel;
		TeamPtr team;
		int channel_index = -1;

		for(int i = 0; i < sc->m_teams.size(); i++ )
		{
			if( scmp(sc->m_teams[i]->m_id,team_id) )
			{
				team = sc->m_teams[i];
				team_index = i;

				QVector<ChannelPtr> *channels;
				if( type == ChannelType::ChannelPublic )
					channels = &team->m_public_channels;
				else if( type == ChannelType::ChannelPrivate )
					channels = &team->m_private_channels;
				if(!channels)
				{
					qWarning() << "Wrong channel type" << type;
					return;
				}
				for(int j = 0; j < channels->size(); j++ )
				{
					if( channels->at(j)->m_id.compare(channel_id) == 0 )
					{
						channel = channels->at(j);
						channel_index = j;
						break;
					}
				}
				break;
			}
		}
		if(team) {
			team->m_unread_messages++;
			if( !mentionsv.isUndefined() ) {
				QJsonDocument d = QJsonDocument::fromJson(mentionsv.toString().toUtf8());
				QJsonArray mentions = d.array();
				for(int m = 0; m < mentions.size(); m++ ) {
					if( !mentions[m].isString() )
						continue;
					if( mentions[m].toString() == m_server[team->m_server_index]->m_user_id ) {
						team->m_unread_mentions++;
						break;
					}
				}
			}
			message->m_team_index = team_index;
			teamChanged(team, QVectorInt() << TeamsModel::RoleUnreadMessageCount << TeamsModel::RoleUnreadMentionCount );
			emit messageUnreadChanged();
		}
		if( channel && channel_index >= 0)
		{
			// TODO refactoring, move all creation parameters to constructor
			message->m_channel_type  = channel->m_type;
			message->m_channel_index = channel_index;
			message->m_self_index    = channel->m_message.size();
			channel->m_message.append(message);
			if(false)
				for(int k = 0; k < message->m_file_ids.size(); k++ )
				{
					if(message->m_type == MessageMine)
					{// search if we already has files
						bool fileDownloaded = false;
						QList<FilePtr>::iterator
						        it = sc->m_sended_files.begin(),
						        end = sc->m_sended_files.end();
						while(it != end)
						{
							FilePtr f = *it;
							if(f->m_id == message->m_file_ids[k] )
							{
								it = sc->m_sended_files.erase(it);
								fileDownloaded = true;
								f->m_self_index = message->m_file.size();
								message->m_file.append(f);
								break;
							}
						}
						if(fileDownloaded)
							continue;
					}
#ifdef PRELOAD_FILE_INFOS
					get_file_info(sc->m_self_index,team_index,type,channel_index,
					              message->m_self_index, message->m_file_ids[k]);
#endif
				}
			QList<MessagePtr> new_messages;
			new_messages << message;
			emit messageAdded(new_messages);// add messages to model
			// chek if messed sended from another user, then
			if( message->m_type != MessageMine )
			{
				// notificaton
				emit newMessage(message);
				// add unread msg count to team
				//				 = m_server[channel->m_server_index]->m_teams[team_index];
				if( mentionsv.isArray() )
				{
					QJsonDocument d = QJsonDocument::fromJson(mentionsv.toString().toUtf8());
					QJsonArray mentions = d.array();
					for(int m = 0; m < mentions.size(); m++ ) {
						if( !mentions[m].isString() )
							continue;
						if( mentions[m].toString() == m_server[team->m_server_index]->m_user_id ) {
							channel->m_mention_count++;
							break;
						}
					}
				}
				channel->m_msg_unread++;
				emit updateChannel(channel, QVector<int>() << ChannelsModel::MessageUnread << ChannelsModel::MentionCount );
				emit messageUnreadChanged();
			}
		}
	}
	if(message->m_channel_index == -1 )
	{//TODO Channel not found, need load channel info from server
		// Need some structure for unnatached messages (when it need)
		//maybe need use only channels list in ServerPtr,
		//and download TeamPtr and teamslist after it needed by user
		//		UMessagePtr umessage(new UnattachedMessageContainer);
		//		umessage->m_team_id = data["team_id"].toString();
		//		umessage->m_message = message;
		//		sc->m_untacched_messages.append(umessage);
		if( message->m_type != MessageMine )
			// notification
			emit newMessage(message);
	}

	// check if message is answer
	if( !message->m_root_id.isEmpty() )
	{
		// here we request root post by id
		get_post(message->m_server_index, message->m_root_id, message);
	}
}

void MattermostQt::event_post_edited(MattermostQt::ServerPtr sc, QJsonObject object)
{
	QJsonObject data = object["data"].toObject();
	//	QJsonObject broadcast = object["broadcast"].toObject();
	//	int team_index = -1;
	ChannelPtr channel;

	QJsonObject post = data["post"].toObject();
	if( post.isEmpty() )
	{
		QJsonValue value = data.value("post");
		if( value.type() == QJsonValue::String )
		{
			QString s = value.toString();
			QJsonDocument j = QJsonDocument::fromJson( s.toUtf8() );
			post = j.object();
			if(post.isEmpty())
				return;
		}
		else
			return;
	}
	MessagePtr message( new MessageContainer(post, sc->m_user_id) );

	// search for channel
	// TODO make na optimized search! maybe use QMap?
	for(int ci = 0; ci < sc->m_direct_channels.size(); ci++ )
	{
		ChannelPtr c = sc->m_direct_channels[ci];
		if( c->m_id == message->m_channel_id )
		{
			channel = c;
			break;
		}
	}
	if(channel.isNull())
		for(int ti = 0; ti < sc->m_teams.size(); ti++ )
		{
			TeamPtr tc = sc->m_teams[ti];
			for(int ci = 0; ci < tc->m_private_channels.size(); ci++ )
			{
				ChannelPtr c = tc->m_private_channels[ci];
				if( c->m_id == message->m_channel_id )
				{
					channel = c;
					break;
				}
			}
			for(int ci = 0; ci < tc->m_public_channels.size(); ci++ )
			{
				ChannelPtr c = tc->m_public_channels[ci];
				if( c->m_id == message->m_channel_id )
				{
					channel = c;
					break;
				}
			}
		}

	//	qDebug() << post;
	//	qDebug() << broadcast;

	if( channel )
	{
		// search for message
		for(int i = 0; i < channel->m_message.size(); i++ )
		{
			MessagePtr mc = channel->m_message[i];
			if( mc->m_id.compare(message->m_id) == 0)
			{
				mc->m_create_at = message->m_create_at;
				mc->m_update_at = message->m_update_at;
				mc->m_message = message->m_message;
				mc->m_formated_message.clear();
				// TODO - check all files (looks like it no need, can change only text )
				//				mc->
				QList<MessagePtr> messages;
				messages << mc;
				emit messageUpdated(messages);
				break;
			}
		}
		return;
	}
}

void MattermostQt::event_status_change(MattermostQt::ServerPtr sc, QJsonObject object)
{
	//qDebug() << data;
	//{"status":"online","user_id":"gqr15ebytjg7znhh4boz74foxy"}
	QJsonObject data  = object["data"].toObject();
	UserStatus status = str2status(data["status"].toString());
	UserPtr current   = id2user(sc,data["user_id"].toString());
	if(!current)
	{// TODO try to send request for user (if it not found)
		// and mark this request (that requested from here)
		return; // FIXME
	}
	current->m_status = status;
	QVectorInt roles;
	roles << UserStatusRole;
	emit userUpdated(current, roles);
}

void MattermostQt::event_typing(MattermostQt::ServerPtr sc, QJsonObject object)
{
	/* {
	 *   "broadcast": {
	 *      "channel_id":"some_channel_id",
	 *      "omit_users":{
	 *        "some user id":true
	 *      },
	 *      "team_id":"",
	 *      "user_id":""
	 *   },"
	 *   "data":{
	 *      "parent_id":"",
	 *      "user_id":"some user id"
	 *   },
	 *   "event":"typing",
	 *   "seq":2
	 * }*/
	QJsonObject data  = object["data"].toObject();
	QJsonObject broadcast = object["broadcast"].toObject();
	//	qDebug() << object;
	QString channel_id = broadcast["channel_id"].toString();
	QString usr_id = data["user_id"].toString();
	ChannelPtr channel = id2channel(sc, channel_id);
	UserPtr user = id2user(sc, usr_id);
	if( channel.isNull() )
	{
		qWarning() << QStringLiteral("Cant find channel id=\"%0\", need requset it from server.").arg(channel_id);
		//		qDebug() << object;
		// TODO here need create another implementation of get channel, or separate get_channel
		// and get_posts for channel (now in response it automatically request posts)
		//		get_channel(sc->m_self_index, channel_id);
		return;
	}

	if( user.isNull() )
	{
		qWarning() << QStringLiteral("User id(%0) stil not fetched from server.").arg(usr_id);
		return;
	}
	user->m_typing = true;
	//	emit
	channel->m_user_typing.push_back(user);
	emit updateChannel(channel, QVectorInt() << ChannelsModel::RoleUsersTyping );

	QTimer::singleShot(5000, this, [this, channel, user]() {
		int index  = channel->m_user_typing.indexOf(user);
		if( index != -1 ) {
			user->m_typing = false;
			channel->m_user_typing.removeAt( index );
			emit updateChannel(channel, QVectorInt() << ChannelsModel::RoleUsersTyping );
		}
	});
}

void MattermostQt::event_channel_viewed(MattermostQt::ServerPtr sc, QJsonObject object)
{
	/*\"data\": {
	 * \"channel_id\":\"dmtkwe7atfbytq3pwm3uikgwmw\"
	 * },
	 * \"broadcast\": {
	 *   \"omit_users\":null,
	 *   \"user_id\":\"gqr15ebytjg7znhh4boz74foxy\",
	 *   \"channel_id\":\"\",
	 *   \"team_id\":\"\"
	 * }
	*/
	QJsonObject data  = object["data"].toObject();
	QString channel_id = data["channel_id"].toString();

	ChannelPtr channel = id2channel(sc, channel_id);

	if(channel.isNull())
	{
		qCritical() << QStringLiteral("Channel not loaded to app, need request data from server.");
		return;
	}

	channel->m_mention_count = 0;
	channel->m_msg_unread = 0;
	get_teams_unread(sc);// TODO should be more effective way
	emit updateChannel(channel, QVectorInt() << ChannelsModel::MessageUnread << ChannelsModel::MentionCount );
}

void MattermostQt::event_reaction_added(MattermostQt::ServerPtr sc, QJsonObject object)
{
	/*{
	 *  "event": "reaction_added",
	 *  "data": {
	 *    "reaction": "{
	 *       \\"user_id\\":\\"gqr15ebytjg7znhh4boz74foxy\\",
	 *       \\"post_id\\":\\"bn7s5xj1xprq7b43e3ahkgi98o\\",
	 *       \\"emoji_name\\":\\"nauseated_face\\",
	 *       \\"create_at\\":1588252042237
	 *    }"
	 *  },
	 *  "broadcast": {
	 *    "omit_users":null,
	 *    "user_id":"",
	 *    "channel_id":"rw7wbmf7638mtgqpnh5p5aih3y",
	 *    "team_id":""
	 *  },
	 *  "seq": 10
	 *}
	 */
	QJsonObject data  = object["data"].toObject();
	QJsonObject broadcast  = object["broadcast"].toObject();

	QString channel_id = broadcast["channel_id"].toString();
	ChannelPtr channel = id2channel(sc, channel_id);

	if(channel.isNull())
	{
		qCritical() << QStringLiteral("Channel not loaded to app, need request data from server.");
		return;
	}
	QString reaction_json = data["reaction"].toString();
	QJsonObject reaction = QJsonDocument::fromJson(reaction_json.toUtf8()).object();
	QString message_id = reaction["post_id"].toString();
	QString emoji_name = reaction["emoji_name"].toString();
	QString user_id = reaction["user_id"].toString();
	MessagePtr message = id2message(channel, message_id);
	if(!message)
	{
		qWarning() << QStringLiteral("Cant find message in channel. Looks it not loaded for view in model, just skip it.");
		return;
	}
	//TODO add reaction to message
	ReactionContainer new_reaction;
	new_reaction.m_emoji = emoji_name;
	new_reaction.m_user_id.append(user_id);
	new_reaction.m_mine_emoji = user_id == sc->m_user_id;
	message->addReaction(new_reaction);
	updateMessage(message, MessagesModel::RoleReactionsCount);
}

void MattermostQt::event_reaction_removed(MattermostQt::ServerPtr sc, QJsonObject object)
{
	/*{
	 *  "event": "reaction_removed",
	 *  "data\": {
	 *    "reaction": "{
	 *       \\"user_id\\":\\"fyrn71gupjdd7e4bh84npwxw8r\\",
	 *       \\"post_id\\":\\"g8e5gpxiaby9jq661o1mq5njgh\\",
	 *       \\"emoji_name\\":\\"rage\\",
	 *       \\"create_at\\":0}"
	 *    },
	 *    "broadcast": {
	 *       "omit_users":null,
	 *       "user_id":"",
	 *       "channel_id":"rw7wbmf7638mtgqpnh5p5aih3y",
	 *       "team_id":""
	 *    },
	 *  "seq": 7
	 *}
	 * */

	QJsonObject data  = object["data"].toObject();
	QJsonObject broadcast  = object["broadcast"].toObject();

	QString channel_id = broadcast["channel_id"].toString();
	ChannelPtr channel = id2channel(sc, channel_id);

	if(channel.isNull())
	{
		qCritical() << QStringLiteral("Channel not loaded to app, need request data from server.");
		return;
	}
	QString reaction_json = data["reaction"].toString();
	QJsonObject reaction = QJsonDocument::fromJson(reaction_json.toUtf8()).object();
	QString message_id = reaction["post_id"].toString();
	QString emoji_name = reaction["emoji_name"].toString();
	QString user_id = reaction["user_id"].toString();
	MessagePtr message = id2message(channel, message_id);
	if(!message)
	{
		qWarning() << QStringLiteral("Cant find message in channel. Looks it not loaded for view in model, just skip it.");
		return;
	}
	//TODO add reaction to message
	ReactionContainer new_reaction;
	new_reaction.m_emoji = emoji_name;
	new_reaction.m_user_id.append(user_id);
	new_reaction.m_mine_emoji = user_id == sc->m_user_id;
	message->removeReaction(new_reaction);
	updateMessage(message, MessagesModel::RoleReactionsCount);
}

MattermostQt::UserStatus MattermostQt::str2status(const QString &s) const
{
	if( s.compare("online") == 0 )
		return UserOnline;
	else if( s.compare("away") == 0 )
		return UserAway;
	else if( s.compare("offline") == 0 )
		return UserOffline;
	else if( s.compare("dnd") == 0 )
		return UserDnd;//do not disturb
	return UserNoStatus;
}

MattermostQt::UserPtr MattermostQt::id2user(ServerPtr sc, const QString &id) const
{
	// is not fast algorith. but now use it
	for(int i = 0, count = sc->m_user.size(); i < count ; i++)
	{
		if( sc->m_user[i]->m_id == id )
			return sc->m_user[i];
	}
	return UserPtr();
}

MattermostQt::ChannelPtr MattermostQt::id2channel(MattermostQt::ServerPtr sc, const QString &channel_id) const
{
	ChannelPtr channel;
	// use qHash
	auto it = sc->m_channels_hash.find(channel_id);
	if( it != sc->m_channels_hash.end() ) {
		return it.value();
	}
	// the channels info stil not requested from server
	// if something went wrong, try find it with many cycles
	for( auto channel : sc->m_direct_channels)
		if( channel->m_id == channel_id )
		{
			channel = channel;
			break;
		}

	if( channel.isNull() )
		for( auto team : sc->m_teams )
		{
			for( int i = 0 ; i < 2; i++ ) {
				QVector<ChannelPtr> *channels = nullptr;
				if( i == 0 )
					channels = &team->m_public_channels;
				else
					channels = &team->m_private_channels;
				// Public channels
				for( auto channel : *channels )
					if( channel->m_id == channel_id )
					{
						channel = channel;
						break;
					}
			}

			if(!channel.isNull())
				break;
		}
	return channel;
}

MattermostQt::MessagePtr MattermostQt::id2message(MattermostQt::ChannelPtr channel, const QString &id) const
{
	QVector<MessagePtr>::iterator it = std::find_if( channel->m_message.begin(), channel->m_message.end(), [id](const MessagePtr &m) {
		return m->m_id == id;
	} );
	if( it != channel->m_message.end() )
		return *it;
	return MessagePtr();
}

const QString MattermostQt::generateCachePath(bool *isOk) const
{
	QString cache_path = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
	if( cache_path.indexOf( QStringLiteral(".cache/%0").arg(QCoreApplication::organizationName()) ) != -1 )
	{	// this wrong path, try fix it
		// its /home/nemo/.local/share/sashikknox/harbour-mattermost
		cache_path.remove( cache_path.lastIndexOf(QCoreApplication::organizationName().append("/")) , QCoreApplication::organizationName().append("/").length() );
		if(isOk)
			*isOk = false;
	}
	if(isOk)
		*isOk = true;
	return cache_path;
}

void MattermostQt::event_post_deleted(MattermostQt::ServerPtr sc, QJsonObject object)
{
	ChannelPtr channel;
	QJsonObject data = object["data"].toObject();
	QJsonObject post = data["post"].toObject();
	if( post.isEmpty() )
	{
		QJsonValue value = data.value("post");
		if( value.type() == QJsonValue::String )
		{
			QString s = value.toString();
			QJsonDocument j = QJsonDocument::fromJson( s.toUtf8() );
			post = j.object();
			if(post.isEmpty())
				return;
		}
		else
			return;
	}
	MessagePtr message( new MessageContainer(post, sc->m_user_id) );

	// search for channel
	channel = id2channel(sc, message->m_channel_id);

	if( channel )
	{
		MessagePtr deleted;
		int index = -1;
		// search for message
		for(int i = 0; i < channel->m_message.size(); i++ )
		{
			MessagePtr mc = channel->m_message[i];
			if( mc->m_id.compare(message->m_id) == 0)
			{
				deleted = mc;
				index = i;
				mc->m_delete_at = message->m_delete_at;
				//mc->m_message = message->m_message;
				QList<MessagePtr> messages;
				messages << mc;

				emit messageBeginDelete( messages );
				channel->m_message.remove(i);
				emit messageDeleted( messages );

				break;
			}
		}
		if(deleted && index >= 0)
		{
			// here we update all messages, with link to this message
			if(!deleted->m_thread_messages.isEmpty())
			{
				for(QList<MattermostQt::MessagePtr>::iterator
				    it = deleted->m_thread_messages.begin(), end = deleted->m_thread_messages.end();
				    it != end; it++ )
				{
					MattermostQt::MessagePtr ans = *it;
					//					ans->m_root_message = tr("Message deleted"); ans->m_root_ptr.reset();
					for(int i = index; i < channel->m_message.size();)
					{
						if( ans == channel->m_message[i] )
						{
							QList<MessagePtr> messages;
							messages << channel->m_message[i];
							channel->m_message[i]->m_self_index = i;
							emit messageBeginDelete( messages );
							channel->m_message.remove(i);
							emit messageDeleted(messages);
						}
						else {
							i++;
						}
					}
				}
			}

			for(int i = index; i < channel->m_message.size(); i++ )
				channel->m_message[i]->m_self_index = i;
			//			deleted->m_thread_messages.prepend(deleted);
			//			emit messageBeginDelete( deleted->m_thread_messages );
			//			emit messageDeleted(deleted->m_thread_messages);
			//			deleted->m_thread_messages.pop_front();

			for(QList<MattermostQt::MessagePtr>::iterator
			    it = deleted->m_thread_messages.begin(), end = deleted->m_thread_messages.end();
			    it != end; it++ )
			{	// nothing to do here
				// check if messages has its own replies
				// (its possible ony by API, but not in official client)
				MessagePtr answer = *it;
				if(!answer->m_thread_messages.isEmpty())
				{
					for(QList<MattermostQt::MessagePtr>::iterator
					    t_it = answer->m_thread_messages.begin(), t_end = answer->m_thread_messages.end();
					    t_it != t_end; t_it++ )
					{
						MessagePtr reply = *t_it;
						reply->m_root_ptr = MessagePtr();
						reply->m_root_id.clear();
						reply->m_root_message.clear();

					}
					emit messageUpdated(answer->m_thread_messages);
				}
			}

			if(deleted->m_root_ptr)
			{
				MessagePtr root_ptr = deleted->m_root_ptr;
				for(QList<MattermostQt::MessagePtr>::iterator
				    it = root_ptr->m_thread_messages.begin(), end = root_ptr->m_thread_messages.end();
				    it != end; it++ )
				{
					MessagePtr current = *it;
					if( current == deleted )
					{
						root_ptr->m_thread_messages.erase(it);
						break;
					}
				}
			}
		}
		return;
	}
}

void MattermostQt::replyFinished(QNetworkReply *reply)
{
	QVariant replyType;
	replyType = reply->property(P_REPLY_TYPE);
#ifdef MQT_TRACE
	qDebug() << "MattermostQt::replyFinished() " << replyType.toInt();
#endif
	if (reply->error() == QNetworkReply::NoError) {
		//success
		//		if(reply->header(QNetworkRequest::LastModifiedHeader).isValid())
		//			qDebug() << "LastModified" << reply->header(QNetworkRequest::LastModifiedHeader);

		if(replyType.isValid())
		{
			int  reply_type = replyType.toInt();
			if( reply_type >= 0 && reply_type < ReplyTypeCount && m_reply_func[reply_type] != nullptr )
			{
				(this->*m_reply_func[reply_type])(reply);
				emit requestFinished((ReplyType)reply_type);
			}
			else
			{
				qWarning() << "Unknown reoply type: " << reply_type;
				qDebug() << "Reply:" << QString::fromUtf8( reply->readAll() );
				QList<QByteArray> headers = reply->rawHeaderList();
				foreach(QByteArray header, headers)
				{
					qDebug() << header;
				}
			}
		}
		else
		{
			qWarning() << "Unknown reply type";
			qDebug() << "Success" << QString::fromUtf8( reply->readAll() );
		}
	}
	else {
		//failure
		qWarning() << "Failure: " << reply->error() << reply->errorString();
		qDebug() << reply;
		reply_error(reply);
		switch(reply->error() )
		{
		case QNetworkReply::AuthenticationRequiredError: {
			// need authentification
			// TODO show error in LoginPage about bad authetificaation
			//qWarning() << reply->;
			break;
		}
		case QNetworkReply::TimeoutError:
		{
			qDebug() << "Reply: " << reply->readAll();

			for(int i = 0; i < m_server.size(); i++)
			{
				ServerPtr server = m_server[i];
				server->m_state = ServerState::ServerUnconnected;
				if(server->m_socket)
					server->m_socket->close(QWebSocketProtocol::CloseCodeAbnormalDisconnection, QString("Client closing") );
				emit serverStateChanged(i, server->m_state);
				server->m_ping_timer.stop();
				qDebug() << QStringLiteral("Server[%0] (%1)  stop ping timer.").arg(server->m_self_index).arg(server->m_display_name);
				m_user_status_timer.stop();
				m_reconnect_server.start();
				qDebug() << QStringLiteral("Server[%0] (%1)  start reconnect timer.").arg(server->m_self_index).arg(server->m_display_name);
			}
			break;
		}
		case QNetworkReply::UnknownNetworkError:
		default:
		{
			for(int i = 0; i < server().size(); i ++ )
			{
				if(server().at(i)->m_socket)
					server().at(i)->m_socket->ping(QStringLiteral("ping").toUtf8());
				else {
					server().at(i)->m_state = ServerState::ServerUnconnected;
					emit serverStateChanged(i, server().at(i)->m_state);
					m_reconnect_server.start();
					qWarning() << QStringLiteral("Unknown network error. Start reconnect timer;");
				}
			}
			break;
		}
		}


		if(replyType.isValid())
		{
			switch(replyType.toInt())
			{
			case ReplyType::rt_get_file_info:
				break;
			case ReplyType::rt_get_user_info:
				break;
			}
		}
	}
	delete reply;
}

void MattermostQt::slotNetworkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility accessible)
{
	// TODO stop all pings, requsets and other, when network is not accessible
	switch(accessible)
	{
	case QNetworkAccessManager::UnknownAccessibility:
	case QNetworkAccessManager::Accessible:
		if( accessible == QNetworkAccessManager::UnknownAccessibility)
			qDebug() << "QNAM accesibility changed: UnknownAccessibility";
		else
			qDebug() << "QNAM accesibility changed: Accessible";
		for(int i = 0; i < m_server.size(); i++)
		{
			ServerPtr server = m_server[i];
			if( server->m_state != ServerConnected ) {
				slot_recconect_servers();
				m_reconnect_server.start();
				qDebug() << QLatin1String("Recconect timer started.");
				m_user_status_timer.start();
			}
		}
		break;
	case QNetworkAccessManager::NotAccessible:
		qDebug() << "QNAM accesibility changed: NotAccessible";
		// TODO here we need of all servers connections
		for(int i = 0; i < m_server.size(); i++)
		{
			ServerPtr server = m_server[i];
			//			server->m_socket->st
			server->m_state = ServerState::ServerUnconnected;
			if(server->m_socket)
				server->m_socket->close(QWebSocketProtocol::CloseCodeAbnormalDisconnection, QString("Client closing") );
			emit serverStateChanged(i, server->m_state);
			server->m_ping_timer.stop();
			qDebug() << QStringLiteral("Server[%0] (%1)  stop ping timer.").arg(server->m_self_index).arg(server->m_display_name);
			m_user_status_timer.stop();
			m_reconnect_server.stop();
			qDebug() << QStringLiteral("Recconect timer stoped.");
		}
		break;
	}
}

void MattermostQt::replySSLErrors(QNetworkReply *reply, QList<QSslError> errors)
{
	bool trustCertificate = false;
	QVariant ts = reply->property(P_SERVER_INDEX);
	if( ts.isValid() )
		trustCertificate = m_server[ts.toInt()]->m_trust_cert;
	else
	{
		QVariant var_ts = reply->property(P_TRUST_CERTIFICATE);
		if( var_ts.isValid() && var_ts.type() == QVariant::Bool )
			trustCertificate = var_ts.toBool();
	}
	if(trustCertificate)
	{
		QList<QSslError> ignoreErrors;
		foreach(QSslError error, errors)
		{
			qWarning() << QLatin1String("SslError")
			           << (int)error.error()
			           << QLatin1String(":")
			           << error.errorString();

			switch( error.error() )
			{
			case QSslError::CertificateUntrusted:
			case QSslError::SelfSignedCertificate:
			case QSslError::SelfSignedCertificateInChain:
				// TODO - remove HostNameMistchmach
				ignoreErrors << error;
				break;
			default:
				ignoreErrors << error;
				break;
			}
		}
		reply->ignoreSslErrors(ignoreErrors);
		//		reply->ignoreSslErrors(errors);
	}
	else
	{
		emit onConnectionError(ConnectionError::SslError, "SSL Error", -1);
	}
}

void MattermostQt::replyDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
	QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
	if(!reply)
		return;
	FilePtr file = reply->property(P_FILE_PTR).value<FilePtr>();
	if(!file)
		return;
	if( bytesTotal <= 0 )
	{
		bytesTotal = file->m_file_size;
	}
	emit fileDownloadingProgress(file->m_id, (qreal)bytesReceived/(qreal)bytesTotal);
}

void MattermostQt::replyUploadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
	QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
	if(!reply)
		return;
	ChannelPtr channel = reply->property(P_CHANNEL_PTR).value<ChannelPtr>();
	QString file_path = reply->property(P_FILE_PATH).toString();
	QString data;
	if(channel)
		data = channel->m_id;
#ifdef DESKTOP_APP
	data += QStringLiteral("; ") + file_path;
#endif
	int progress = int((qreal)bytesReceived/(qreal)bytesTotal*100.0);
	emit fileUploadProgress(data,progress);
}

void MattermostQt::onWebSocketConnected()
{
	QWebSocket * socket = qobject_cast<QWebSocket*>(sender());
	if(!socket) // strange situation, if it happens
		return;

	QVariant sId = socket->property(P_SERVER_INDEX);
	if(!sId.isValid()) // that too strange!!! that cant be!
		return;

	int server_index = sId.toInt();
	if( server_index < 0 || server_index >= m_server.size() )
		return;
	ServerPtr sc = m_server[server_index];

	//	{
	//	  "seq": 1,
	//	  "action": "authentication_challenge",
	//	  "data": {
	//	    "token": "mattermosttokengoeshere"
	//	  }
	//	}
	QJsonDocument json;
	QJsonObject root;
	QJsonObject data;

	data["token"] = sc->m_token;
	root["seq"] = 1;
	root["action"] = QString("authentication_challenge");
	root["data"] = data;
	json.setObject(root);

	sc->m_socket->sendTextMessage(json.toJson().data());
	sc->m_ping_timer.setProperty(P_SERVER_INDEX, server_index);
	connect(&sc->m_ping_timer, SIGNAL(timeout()), this, SLOT(slot_ping_timeout()));
	sc->m_ping_timer.start();
	//	emit serverConnected(server_index);
}

void MattermostQt::onWebSocketSslError(QList<QSslError> errors)
{
	QList<QSslError> ignoreErrors;
	foreach(QSslError error, errors)
	{
		qWarning() << QLatin1String("SslError")
		           << error
		           << QLatin1String(":")
		           << error.errorString();

		switch( error.error() )
		{
		case QSslError::CertificateUntrusted:
		case QSslError::SelfSignedCertificate:
		case QSslError::SelfSignedCertificateInChain:
			// TODO - remove HostNameMistchmach
			ignoreErrors << error;
			break;
		default:
			ignoreErrors << error;
			break;
		}
	}
	QWebSocket *socket = qobject_cast<QWebSocket*>(sender());
	if(socket)
	{
		socket->ignoreSslErrors(ignoreErrors);
		if(!ignoreErrors.empty()) {
			// try set status to Connectiong, for beauty loading process in UI
			QVariant sId = socket->property(P_SERVER_INDEX);
			if(!sId.isValid()) {
				qCritical() << "Cant get server's Id (index) from QWebSocket pointer";
				return;
			}
			int server_index = sId.toInt();
			if( server_index < 0 || server_index >= m_server.size() ) {
				qCritical() << QStringLiteral("Cant get server from index %0").arg(server_index);
				return;
			}
			ServerPtr sc = m_server[server_index];
			sc->m_state = (int)QAbstractSocket::ConnectingState;
			emit serverStateChanged(server_index, (int)sc->m_state);
		}
	}
	//	qDebug() << err;
}

void MattermostQt::onWebSocketError(QAbstractSocket::SocketError error)
{
	QWebSocket * socket = qobject_cast<QWebSocket*>(sender());
	if(!socket) {
		// strange situation, if it happens
		qWarning() << "WebSocket error: " << error;
		return;
	}
	QString message = QStringLiteral("WebSocketError (%0) : %1")
	        .arg( QVariant::fromValue<QAbstractSocket::SocketError>(error).toString() )
	        .arg( socket->errorString() );

	QVariant sId = socket->property(P_SERVER_INDEX);
	if(!sId.isValid()) // that too strange!!! that cant be!
	{
		qWarning() << message;
		return;
	}
	int server_index = sId.toInt();

	switch(error)
	{
	case QAbstractSocket::SslHandshakeFailedError:
	{
		if( server_index < 0 || server_index >= m_server.size() )
			break;
		ServerPtr server = m_server[server_index];

		if(server->m_trust_cert) {
			qInfo() << QStringLiteral("WebSocketError: SslHandshakeFailedError as expected.");
			server->m_state = (int)QAbstractSocket::ConnectingState;
			emit serverStateChanged(server_index, (int)server->m_state);
		}
		else {
			qCritical() << QStringLiteral("WebSocketError: SslHandshakeFailedError check servers certificate.");
			emit onConnectionError(ConnectionError::SslError, message, server_index);
		}
		return;
	}
		break;
	case QAbstractSocket::NetworkError:
	case QAbstractSocket::SocketTimeoutError :
		// should i reconnect by my self, or its automatically ?
	{
		if( server_index < 0 || server_index >= m_server.size() )
			break;
		ServerPtr server = m_server[server_index];

		//server->m_state = ServerState::ServerUnconnected;
		emit serverStateChanged(server_index, server->m_state);
		server->m_ping_timer.stop();
		qDebug() << QStringLiteral("Server[%0] (%1)  stop ping timer.").arg(server->m_self_index).arg(server->m_display_name);
		m_user_status_timer.stop();
		m_reconnect_server.start();
		qDebug() << QStringLiteral("Reconnect timer started.");
		server->m_socket->close(QWebSocketProtocol::CloseCodeAbnormalDisconnection, QString("Client closing") );
		emit onConnectionError(error, message, server_index);
	}
		break;
	default:
		emit onConnectionError(error, message, server_index);
	}
	qWarning() << message;
}

void MattermostQt::onWebSocketStateChanged(QAbstractSocket::SocketState state)
{
	QWebSocket * socket = qobject_cast<QWebSocket*>(sender());
	if(!socket) // strange situation, if it happens
	{
		qCritical() << "Sender is not WebSocket! but WebSocket state changed: "<< state;
		return;
	}
	QVariant sId = socket->property(P_SERVER_INDEX);
	if(!sId.isValid()) {
		qCritical() << "Cant get server's Id! WebSocket state changed: "<< state;
		return;// that too strange!!! that cant be!
	}
	int server_index = sId.toInt();
	//	QStringLiteral s_state;

	if( server_index < 0 || server_index >= m_server.size() ) {
		qCritical() << QStringLiteral("Cant get server from index %0").arg(server_index);
		qDebug() << QStringLiteral("WebSocket [%0] state changed: ").arg(server_index) << state;
		return;
	}
	ServerPtr sc = m_server[server_index];
	switch(state) {
	case QAbstractSocket::UnconnectedState:
		//		s_state = QStringLiteral("UnconnectedState");
		m_reconnect_server.start();
		qDebug() << QLatin1String("Recconect timer started.");
		sc->m_state = (int)state;
		emit serverStateChanged(server_index, (int)sc->m_state);
		break;
	case QAbstractSocket::ConnectingState:
		//		s_state = QStringLiteral("ConnectingState");
		sc->m_state = (int)state;
		emit serverStateChanged(server_index, (int)sc->m_state);
		break;
	case QAbstractSocket::ConnectedState:
	{
		//			s_state = QStringLiteral("ConnectedState");
		sc->m_state = (int)state;
		bool need_reconnect = false;
		for(int i = 0; i < m_server.size(); i++ )
		{
			if( m_server[i]->m_state == ServerUnconnected )
				need_reconnect = true;
			emit serverStateChanged(i, (int)sc->m_state);
		}
		if(!need_reconnect) {
			m_reconnect_server.stop();
			qDebug() << QLatin1String("Recconect timer stoped.");
		}
	}
		break;
	case QAbstractSocket::HostLookupState:
		//		s_state = QStringLiteral("HostLookupState");
		break;
	case QAbstractSocket::BoundState:
		//		s_state = QStringLiteral("BoundState");
		break;
	case QAbstractSocket::ListeningState:
		//		s_state = QStringLiteral("ListeningState");
		break;
	case QAbstractSocket::ClosingState:
		//		s_state = QStringLiteral("ClosingState");
		sc->m_state = (int)ServerUnconnected;
		emit serverStateChanged(server_index, (int)sc->m_state);
		break;
	}
	qDebug() << QStringLiteral("WebSocket [%0] state changed: ").arg(server_index) << state;
}

void MattermostQt::onWebSocketTextMessageReceived(const QString &message)
{
	//	qDebug() << message;

	QWebSocket * socket = qobject_cast<QWebSocket*>(sender());
	if(!socket) // strange situation, if it happens
		return;
	QVariant sId = socket->property(P_SERVER_INDEX);
	if(!sId.isValid()) // that too strange!!! that cant be!
		return;
	int server_index = sId.toInt();
	//	QMap<int,ServerPtr>::iterator it = m_server.find(server_index);
	//	if( it == m_server.end() ) // that really strange! whats wrong? how it could be?
	//		return;
	ServerPtr sc = m_server[server_index];
	QJsonDocument json = QJsonDocument::fromJson(message.toUtf8());
	QJsonObject object = json.object();
	QString event = object["event"].toString();
	//	QJsonObject data = object["data"].toObject();
	QJsonObject broadcast = object["broadcast"].toObject();

	_compare(hello) // that mean we are logged in
	{
		//qDebug() << event;
		save_settings();
		//m_user_status_timer.setSingleShot(false);
		m_user_status_timer.start();
		emit serverConnected(sc->m_self_index);
	}
	else if (!event.isEmpty() )
	{
		bool skip_event = false;
		if( !broadcast.isEmpty() ) {
			QJsonObject omit_users = broadcast["omit_users"].toObject();
			if( !omit_users.isEmpty() ) {
				QStringList ids = omit_users.keys();
				auto it = omit_users.constBegin(), end = omit_users.constEnd();
				for(/*auto it = ids.constBegin(), end = ids.constEnd()*/; it != end; it++ )
				{
					//QJsonObject current = *it;
					if( it.key() == sc->m_user_id ) {
						if( (*it).toBool(false) ) {
							skip_event = true;
							break;
						}
					}
				}
			}
		}
		if ( !skip_event )
		{
			QVariant variant(QStringLiteral("et_") + event);
			int event_num = variant.value<EventType>();
			if ( event_num > 0 && event_num < EventTypeCount && m_event_func[event_num] != 0 )
			{
				qDebug() << QStringLiteral("Run event handler for: %0").arg(event);
				(this->*m_event_func[event_num])(sc, object);
			}
			else {
				qWarning() << QStringLiteral("Unhandled event(%0) : %1").arg(event).arg(message);
			}
		}
	}
	else {
		qWarning() << QStringLiteral("Unhandled web socket text message: %0").arg(message);
	}

	/** that need release first */
	//response
	//channel_created
	//channel_deleted
	//channel_updated
	//direct_added
	//group_added

	//leave_team
	//update_team
	//delete_team
	//reaction_added
	//reaction_removed
	//emoji_added

	//ephemeral_message
	//events
	//added_to_team
	//new_user
	//user_added
	//user_updated
	//user_role_updated
	//memberrole_updated
	//user_removed
	//preference_changed
	//preferences_changed
	//preferences_deleted
	//webrtc
	//authentication_challenge
	//license_changed
	//config_changed
}

void MattermostQt::onWebSocketPong(quint64 elapsedTime, QByteArray payload)
{
	// here is we can logging pings
	QWebSocket * socket = qobject_cast<QWebSocket*>(sender());
	if(!socket) // strange situation, if it happens
		return;
	QVariant sId = socket->property(P_SERVER_INDEX);
	if(!sId.isValid()) // that too strange!!! that cant be!
		return;
	int server_index = sId.toInt();
	if( server_index < 0 || server_index >= m_server.size() )
	{
		qCritical() << "Wrong server index onWebSocketPong;";
		return;
	}
	ServerPtr sc = m_server[server_index];
	if(m_settings && m_settings->debug())
		qDebug() << "All right, server is online";
}

void MattermostQt::slot_get_teams_unread()
{
	foreach(ServerPtr server, m_server)
	{
		get_teams_unread(server);
	}
}

void MattermostQt::slot_recconect_servers()
{
	//	bool stop = false;
	for(int i = 0; i < m_server.size(); i ++ )
	{
		if( !m_server[i]->m_enabled )
			continue;
		if( !m_server[i]->m_socket ) {
			get_login(m_server[i]);
		}
		else if( m_server[i]->m_state != ServerConnected
		         && m_server[i]->m_state != ServerConnecting )
		{
			QString urlString = QLatin1String("/api/v")
			        + QString::number(m_server[i]->m_api)
			        + QLatin1String("/websocket");

			QString serUrl = m_server[i]->m_url;
			serUrl.replace("https://","wss://")
			        .replace("http://","ws://");
			QUrl url(serUrl);
			url.setPath(url.path() + urlString);
			// disable forcing websocket port to 443
//			if( url.toString().indexOf("wss://") >= 0 && (url.port() == 80 || url.port() == -1) ) {
//				url.setPort(443);
//				qWarning() << QStringLiteral("Force set port to 443 for WebSocket connection");
//			}
			m_server[i]->m_socket->open(url);
		}
	}
}

void MattermostQt::slot_user_status()
{
	for(int i = 0; i < m_server.size(); i ++ )
	{
		post_users_status(i);
		if(server().at(i)->m_socket)
			server().at(i)->m_socket->ping(QString("ping").toUtf8());
	}
}

void MattermostQt::slot_ping_timeout()
{
	QTimer *timer = qobject_cast<QTimer*>(sender());
	if(!timer) {
		qCritical() << "Wrong sender!";
		return;
	}
	//	int server_index = timer->userData(Qt::UserRole)->
	ServerPtr server;
	QVariant sin = timer->property(P_SERVER_INDEX);
	bool ok;
	int server_index = sin.toInt(&ok);
	if(ok && (server_index < 0 || server_index >= m_server.size() || !sin.isValid() ))
		ok = false;
	if( ok )
	{
		server = m_server[server_index];
	}
	else
	{
		qWarning() << "Cant get server index, try to find it";

		for(int i = 0; i < m_server.size(); i++)
		{
			if( &m_server[i]->m_ping_timer == timer )
			{
				server = m_server[i];
				break;
			}
		}

		if( server.isNull() )
		{
			qCritical() << "Cant find server pointer for ping;";
			return;
		}
	}

	if( server->m_socket.isNull() )
	{
		qWarning() << "No exists web socket!";
		// TODO maby need create it and connect?
		return;
	}
	if(m_settings && m_settings->debug())
		qDebug() << QStringLiteral("Send ping request by websocket to %0 (%1)").arg(server->m_display_name).arg(server->m_url);
	server->m_socket->ping();
}

void MattermostQt::slot_settingsChanged()
{
	save_settings();
}

void MattermostQt::slot_channelAdded(ChannelPtr channel)
{
	ServerPtr server = m_server[channel->m_server_index];
	server->m_channels_hash.insert(channel->m_id,channel);
	get_channel_unread(channel);
}

MattermostQt::TeamContainer::TeamContainer(QJsonObject &object) noexcept
{
	m_id = object["id"].toString();
	m_create_at = (qlonglong)object["create_at"].toDouble();
	m_update_at = (qlonglong)object["update_at"].toDouble();
	m_delete_at = (qlonglong)object["delete_at"].toDouble();
	m_display_name = object["display_name"].toString();
	m_name = object["name"].toString();
	m_description = object["description"].toString();
	m_email = object["email"].toString();
	m_type = object["type"].toString();
	m_allowed_domains = object["allowed+domains"].toString();
	m_invite_id = object["invite_id"].toString();
	m_allowed_open_invite = object["allow_open_invite"].toBool();

	qDebug() << m_display_name << ":" << m_id;
}

bool MattermostQt::TeamContainer::save_json(QString server_dir_path) const
{// server_dir_path ~/.config/{mattermost_config_dir}/{server_dir}/
	QString path = server_dir_path + QDir::separator()
	        + QLatin1String("teams") + QDir::separator()
	        + m_id + QDir::separator();
	QDir dir;
	if(!dir.exists(path))
		dir.mkpath(path);
	QString info_file = path + QLatin1String("info.json");

	QJsonObject root;
	root["id"] = m_id;
	root["display_name"] = m_display_name;
	root["name"] = m_name;
	root["description"] = m_description;
	root["type"] = m_type;
	root["email"] = m_email;
	root["invite_id"] = m_invite_id;
	root["allowed_domains"] = m_allowed_domains;
	root["allowed_open_invite"] = m_allowed_open_invite;
	root["create_at"] = (double)m_create_at;
	root["update_at"] = (double)m_update_at;
	root["delete_at"] = (double)m_delete_at;

	root["server_index"] = m_server_index; /**< server index in QVector */
	root["self_index"] = m_self_index;   /**< self index in vector */

	QJsonArray public_channels;
	QJsonArray private_channels;
	for(int i = 0; i < m_public_channels.size(); i++)
	{
		if(m_public_channels[i]->save_json(server_dir_path))
			public_channels.append( m_public_channels[i]->m_id );
	}
	root["public_channels"] = public_channels;
	for(int i = 0; i < m_private_channels.size(); i++)
		private_channels.append( m_private_channels[i]->m_id );
	root["private_channels"] = private_channels;

	QJsonDocument json;
	json.setObject(root);

	QFile file(info_file);
	if( file.open(QIODevice::WriteOnly) )
		file.write( json.toJson() );
	else
		return false;
	file.close();
	return true;
}

bool MattermostQt::TeamContainer::load_json(QString server_dir_path)
{// server_dir_path ~/.config/{mattermost_config_dir}/{server_dir}/
	QString path = server_dir_path + QDir::separator()
	        + QLatin1String("teams") + QDir::separator()
	        + m_id + QDir::separator();
	QDir dir;
	if(!dir.exists(path))
		return false;
	QString info_file = path + QLatin1String("info.json");
	QFile file(info_file);
	QJsonDocument json;
	if( !file.open(QIODevice::ReadOnly) )
		return false;
	json = QJsonDocument::fromJson( file.readAll() );
	file.close();

	QJsonObject root;

	m_id = root["id"].toString();
	m_display_name = root["display_name"].toString();
	m_name = root["name"].toString();
	m_description = root["description"].toString();
	m_type = root["type"].toString();
	m_email = root["email"].toString();
	m_invite_id = root["invite_id"].toString();
	m_allowed_domains = root["allowed_domains"].toString();
	m_allowed_open_invite = root["allowed_open_invite"].toBool();
	m_create_at = (qlonglong)root["create_at"].toDouble();
	m_update_at = (qlonglong)root["update_at"].toDouble();
	m_delete_at = (qlonglong)root["delete_at"].toDouble();

	m_server_index = (int)root["server_index"].toDouble();
	m_self_index = (int)root["self_index"].toDouble();

	// TODO make Channels loading
	QVector<ChannelPtr> m_public_channels;
	QVector<ChannelPtr> m_private_channels;

	return true;
}

MattermostQt::ChannelContainer::ChannelContainer(QJsonObject &object) noexcept
{
	//"id": "string",
	//"create_at": 0,
	//"update_at": 0,
	//"delete_at": 0,
	//"team_id": "string",
	//"type": "string",
	//"display_name": "string",
	//"name": "string",
	//"header": "string",
	//"purpose": "string",
	//"last_post_at": 0,
	//"total_msg_count": 0,
	//"extra_update_at": 0,
	//"creator_id": "string"
	m_id = object["id"].toString();
	//"create_at": 0,
	m_update_at = (qlonglong)object["update_at"].toDouble();
	//"delete_at": 0,
	m_team_id = object["team_id"].toString();
	QString ct = object["type"].toString();
	if( ct.compare("O") == 0 )
		m_type = MattermostQt::ChannelPublic;
	else if( ct.compare("P") == 0 )
		m_type = MattermostQt::ChannelPrivate;
	else if( ct.compare("D") == 0 )
		m_type = MattermostQt::ChannelDirect;
	m_display_name = object["display_name"].toString();
	m_name = object["name"].toString();
	m_header = object["header"].toString();
	m_purpose = object["purpose"].toString();
	m_last_post_at = (qlonglong)object["last_post_at"].toDouble();
	m_total_msg_count = (qlonglong)object["total_msg_count"].toDouble();
	m_extra_update_at = (qlonglong)object["extra_update_at"].toDouble();
	m_creator_id = object["creator_id"].toString();
	m_dc_user_index = -1;
	m_msg_unread = 0;
	m_mention_count = 0;
}

bool MattermostQt::ChannelContainer::save_json(QString server_dir_path) const
{
	// TODO

	return true;
}

bool MattermostQt::ChannelContainer::load_json(QString server_dir_path)
{
	// TODO

	return true;
}

MattermostQt::MessagePtr MattermostQt::ChannelContainer::messageAt(int message_index)
{
	if(message_index < 0 || message_index >= m_message.size())
		return MessagePtr();
	return m_message[message_index];
}

MattermostQt::UserContainer::UserContainer(QJsonObject object)
{
	m_status = UserNoStatus;
	//"id": "string",
	m_id = object["id"].toString();
	//"create_at": 0,
	//"update_at": 0,
	m_update_at = (qlonglong)object["update_at"].toDouble();
	//"delete_at": 0,
	//"username": "string",
	m_username = object["username"].toString();;
	//"first_name": "string",
	m_first_name = object["first_name"].toString();
	//"last_name": "string",
	m_last_name = object["last_name"].toString();
	//"nickname": "string",
	m_nickname = object["nickname"].toString();
	//"email": "string",
	//"email_verified": true,
	//"auth_service": "string",
	//"roles": "string",
	memset(m_roles, false, UserSystemRolesCount * sizeof(bool) );

	// grab roles for user
	QJsonValue roles_obj = object["roles"];
	if( roles_obj.isString() )
	{
		QStringList roles = roles_obj.toString().split(' ');
		for(QStringList::iterator i = roles.begin(); i != roles.end(); i++)
		{
			QString role = *i;
			if( role == "system_user") {
				m_roles[SystemUser] = true;
			}
			else if (role == "system_admin") {
				m_roles[SystemAdmin] = true;
			}
		}
	}
	//"locale": "string",
	m_locale = object["locale"].toString();
	//"notify_props": {
	//  "email": "string",
	//  "push": "string",
	//  "desktop": "string",
	//  "desktop_sound": "string",
	//  "mention_keys": "string",
	//  "channel": "string",
	//  "first_name": "string"
	//},
	//"props": { },
	m_last_password_update = (qlonglong)object["last_password_update"].toDouble();
	m_last_picture_update = (qlonglong)object["last_picture_update"].toDouble();
	m_last_activity_at = 0;
	//"failed_attempts": 0,
	//"mfa_active": true
}

MattermostQt::ServerContainer::ServerContainer(QString url, QString token, int api)
{
	m_url = url;
	m_api = api;
	m_token = token;
	m_bearer = bearer_string(token);
	m_trust_cert = false;
	m_ping_timer.setSingleShot(false);
	m_enabled = true;
}

MattermostQt::ServerContainer::~ServerContainer()
{
	if( m_socket ){
		m_socket->blockSignals(true);
		m_socket->close(QWebSocketProtocol::CloseCodeGoingAway, QString("Client closing") );
		m_socket->blockSignals(false);
	}
}

int MattermostQt::ServerContainer::get_team_index(QString team_id)
{
	for(int i = 0; i < m_teams.size(); i++)
	{
		TeamPtr tc = m_teams[i];
		if( tc->m_id.compare(team_id) == 0 )
			return i;
	}
	return -1;
}

MattermostQt::MessageContainer::MessageContainer() noexcept
{
	m_user_index = -1;
	m_server_index = -1;
	m_channel_index  = -1;
	m_team_index = -1;
	m_root_user_index = -1;
	m_item_height = 16;
}

MattermostQt::MessageContainer::MessageContainer(QJsonObject object, const QString &user_id)
{
	m_user_index = -1;
	m_server_index = -1;
	m_channel_index  = -1;
	m_team_index = -1;
	m_root_user_index = -1;
	m_item_height = 16;
//	qDebug() << object;
	m_id = object["id"].toString();
	m_channel_id = object["channel_id"].toString();
	m_message = object["message"].toString();
	m_type_string = object["type"].toString();
	m_user_id = object["user_id"].toString();
	m_root_id = object["root_id"].toString();
	m_parent_id = object["parent_id"].toString();
	m_original_id = object["original_id"].toString();
//	if( !m_root_id.isEmpty() ) {
//		qDebug() << "Reply message:";
//		qDebug() << object;
//	}
	m_create_at = (qlonglong)object["create_at"].toDouble(0);
	m_update_at = (qlonglong)object["update_at"].toDouble(0);
	m_delete_at = (qlonglong)object["delete_at"].toDouble(0);
	if( m_type_string.indexOf("system_") >= 0 )
		m_type = MessageOwner::MessageSystem;
	else if ( m_user_id == user_id )
		m_type = MessageOwner::MessageMine;
	else
		m_type = MessageOwner::MessageOther;
	QJsonArray filenames = object["filenames"].toArray();
	QJsonArray file_ids = object["file_ids"].toArray();
	for(int i = 0; i < filenames.size(); i++ )
		m_filenames.append( filenames.at(i).toString() );
	for(int i = 0; i < file_ids.size(); i++ )
		m_file_ids.append( file_ids.at(i).toString() );

	QJsonValue v = object["metadata"];
	if( !v.isNull() ) {
		if(v.isObject())
		{
			QJsonObject metadata = v.toObject();
			if( !metadata.keys().isEmpty() )
			{
				qDebug() << "Message" << m_id << "has metadata:" << metadata.keys();
				QJsonValue v_reactions = metadata["reactions"];
				if( v_reactions.isArray() )
				{ // this post has reactions
					QJsonArray reactions = v_reactions.toArray();
					for( int i = 0; i < reactions.size(); i++)
					{
						QJsonObject reaction = reactions[i].toObject();
						QString emoji_name = reaction["emoji_name"].toString();
						QString reaction_user_id = reaction["user_id"].toString();
						QString emoji_path;
						QByteArray name = emoji_name.toUtf8();
						const char* _emoji_path = CppHash::instance()->find_emoji_path(name.data(),name.size());
						if( _emoji_path != nullptr ) {
							emoji_path = QString("file://") + QString::fromUtf8(_emoji_path);
						}
						for( int index = 0; index < m_reactions.size(); index++ )
						{
							if( m_reactions[index].m_emoji ==  emoji_name ) {
								m_reactions[index].m_user_id.push_back( reaction_user_id );
								if( !m_reactions[index].m_mine_emoji )
								{
									m_reactions[index].m_mine_emoji = reaction_user_id == user_id;
								}
								emoji_name.clear();
								break;
							}
						}
						if(emoji_name.isEmpty())
							continue;
						ReactionContainer new_reaction;
						new_reaction.m_user_id.push_back(reaction_user_id);
						new_reaction.m_emoji = emoji_name;
						new_reaction.m_path = emoji_path;
						new_reaction.m_mine_emoji = user_id == reaction_user_id;
						m_reactions.append( new_reaction );
					}
				}
			}
		}
		else {
			qDebug() << v;
		}
	}
}

bool MattermostQt::MessageContainer::addReaction(ReactionContainer &reaction)
{
	bool result = true;
	// check if already has same reaction
	for( int index = 0; index < m_reactions.size(); index++ )
	{
		if( m_reactions[index].m_emoji == reaction.m_emoji ) {
			// try check if reaction is already there
			if( std::find( m_reactions[index].m_user_id.begin(),
			               m_reactions[index].m_user_id.end(),
			               reaction.m_user_id[0]) == m_reactions[index].m_user_id.end() )
				m_reactions[index].m_user_id.append( reaction.m_user_id[0] );
			else
				qWarning() << QStringLiteral("Reaction %0 already in message id(%1) user id(%2)%3")
				        .arg(reaction.m_emoji)
				        .arg(m_id)
				        .arg( reaction.m_mine_emoji ? QStringLiteral(" (this you)") : QStringLiteral("") );

			if( reaction.m_mine_emoji )
				m_reactions[index].m_mine_emoji = true;
			return result;
		}
	}


	QByteArray name = reaction.m_emoji.toUtf8();
	const char* emoji_path = CppHash::instance()->find_emoji_path(name.data(),name.size());
	if( emoji_path != nullptr ) {
		reaction.m_path = QString("file://") +QString::fromUtf8(emoji_path);
	}
	else {
		result = false;
	}

	m_reactions.append(reaction);
	return result;
}

bool MattermostQt::MessageContainer::removeReaction(const ReactionContainer &reaction)
{
	bool result = true;

	// check if already has same reaction
	for( int index = 0; index < m_reactions.size(); index++ )
	{
		if( m_reactions[index].m_emoji == reaction.m_emoji )
		{
			for( int ui = 0; ui < m_reactions[index].m_user_id.size(); ui ++ )
			{
				if( m_reactions[index].m_user_id[ui] == reaction.m_user_id[0] )
				{
					m_reactions[index].m_user_id.remove(ui);
					if( m_reactions[index].m_user_id.empty() )
						m_reactions.remove(index);
					else if(m_reactions[index].m_mine_emoji && reaction.m_mine_emoji) {
						m_reactions[index].m_mine_emoji = false;
					}
					return result;
				}
			}
		}
	}

	qWarning() << QStringLiteral("Cant find reaction %0 in message id(%1)").arg(reaction.m_emoji).arg(m_id);
//	m_reactions_paths.append( emoji_name);
//	m_reactions_count.append(1);
	return result;
}

bool MattermostQt::MessageContainer::updateRootMessage( MattermostQt *mattermost )
{
//	qInfo() << "MattermostQt::MessageContainer::updateRootMessage";
	if(m_root_ptr.isNull())
	{
		return false;
	}
	if( !m_root_ptr->m_message.isEmpty() )
	{
		m_root_message = SettingsContainer::strToSingleLine(m_root_ptr->m_message);
	}
	else if( !m_root_ptr->m_file.isEmpty() ) // make root message from filenames
	{
		if( m_root_ptr->m_file[0]->m_file_type == FileAnimatedImage ||  m_root_ptr->m_file[0]->m_file_type == FileImage )
			m_root_message = tr("Image: ") + m_root_ptr->m_file[0]->m_name;
		else
			m_root_message = tr("File: ") + m_root_ptr->m_file[0]->m_name;
	}
	else
	{
		m_root_message = tr("Empty message");
	}

	if( (m_root_user_index == -1 || m_root_user_name.isEmpty()) )
	{
		MattermostQt::UserPtr user = mattermost->userAt(m_root_ptr->m_server_index, m_root_ptr->m_user_index);
		if( user ) {
			m_root_user_index = m_root_ptr->m_user_index;
			m_root_user_name = user->m_username;
		}
		else {
			m_root_user_index = -1;
		}
	}
//	qInfo() << "MattermostQt::MessageContainer::updateRootMessage end";
	return true;
}

MattermostQt::FilePtr MattermostQt::MessageContainer::fileAt(int file_index)
{
	if( file_index < 0 || file_index >= m_file.size() )
		return MattermostQt::FilePtr();
	return m_file[file_index];
}

MattermostQt::FileContainer::FileContainer() noexcept
    : m_file_status(FileUninitialized)
    , m_self_sc_index(-1)
    , m_self_index(-1)
    , m_server_index(-1)
    , m_channel_index(-1)
    , m_message_index(-1)
    , m_is_in_cache(2)
{

}

MattermostQt::FileContainer::FileContainer(QJsonObject object) noexcept
    : m_file_status(FileUninitialized)
    , m_self_sc_index(-1)
    , m_self_index(-1)
    , m_server_index(-1)
    , m_channel_index(-1)
    , m_message_index(-1)
    , m_is_in_cache(2)
{
	parse_from_json(object);
}

bool MattermostQt::FileContainer::save_json(QString server_data_path) const
{
	QString fpath = server_data_path + QDir::separator()
	        + QLatin1String("files") + QDir::separator()
	        + m_id;
	QDir dir(fpath);
	if( !dir.exists() )
		dir.mkpath(fpath);
	fpath += QDir::separator() + QLatin1String("info.json");
	QFile conf(fpath);
	conf.setPermissions(QFile::WriteOwner | QFile::ReadOwner);
	if( !conf.open(QFile::WriteOnly) )
		return false;
	QJsonDocument json;
	QJsonObject obj;
	obj["name"] = m_name;
	obj["file_size"] =  double(m_file_size);
	obj["thumb_path"] = m_thumb_path;
	obj["file_type"] = (int)m_file_type;
	obj["file_path"] = m_file_path;
	obj["preview_path"] = m_preview_path;
	QJsonObject size;
	size["width"] = m_item_size.width();
	size["height"] = m_item_size.height();
	obj["item_size"] = size;
	size["width"] = m_image_size.width();
	size["height"] = m_image_size.height();
	obj["image_size"] = size;
	obj["content_width"] = m_contentwidth;

	obj["post_id"] = m_post_id;
	obj["user_id"] = m_user_id;
	obj["mime_type"] = m_mime_type;
	obj["has_preview_image"] = m_has_preview_image ;
	obj["extension"] = m_extension;

	json.setObject(obj);
	conf.write( json.toJson() );
	conf.flush();
	conf.close();
	return true;
}

bool MattermostQt::FileContainer::load_json(QString server_data_path)
{
	QString fpath = server_data_path + QDir::separator()
	        + QLatin1String("files") + QDir::separator()
	        + m_id;
	fpath += QDir::separator() + QLatin1String("info.json");
	QFile conf(fpath);
	//conf.setPermissions(QFile::WriteOwner | QFile::ReadOwner);
	if( !conf.open(QFile::ReadOnly) )
		return false;
	QJsonDocument json = QJsonDocument::fromJson(conf.readAll());
	conf.close();
	if( json.isEmpty() )
		return false;

	QJsonObject obj = json.object();
	m_name = obj["name"].toString();
	m_file_size = qlonglong(obj["file_size"].toDouble());
	m_thumb_path = obj["thumb_path"].toString();
	m_file_type = (MattermostQt::FileType)obj["file_type"].toInt();
	m_file_path = obj["file_path"].toString();
	m_preview_path = obj["preview_path"].toString();
	QJsonObject item_size = obj["item_size"].toObject();
	m_item_size.setWidth(item_size["width"].toDouble());
	m_item_size.setHeight(item_size["height"].toDouble());
	m_contentwidth = obj["content_width"].toInt();

	m_post_id = obj["post_id"].toString("");
	m_user_id = obj["user_id"].toString("");
	m_mime_type = obj["mime_type"].toString("");
	m_has_preview_image = obj["has_preview_image"].toBool(false);
	m_extension = obj["extension"].toString("");

	if( m_file_path.isEmpty() || !QFile::exists(m_file_path) )
	{
		m_file_path.clear();
		m_file_status = FileStatus::FileRemote;
	}
	else
		m_file_status = FileStatus::FileDownloaded;
	bool result = true;
	if( m_file_type == FileType::FileImage || m_file_type == FileType::FileAnimatedImage )
	{
		if( obj.contains("image_size") )
		{
			QJsonObject image_size = obj["image_size"].toObject();
			m_image_size.setWidth( image_size["width"].toInt() );
			m_image_size.setHeight( image_size["height"].toInt() );
		}
		else {
			m_image_size = QSize(-1,-1);
		}

		if( m_thumb_path.isEmpty() || !QFile::exists(m_thumb_path) )
		{
			m_thumb_path.clear();
			result = false;
		}
		if( !m_has_preview_image || m_preview_path.isEmpty() || !QFile::exists(m_preview_path) )
		{
			m_has_preview_image = false;
			m_preview_path.clear();
			result = false;
		}
	}

	if(m_post_id.isEmpty())
		return false;
	if(m_user_id.isEmpty())
		return false;
	if(m_name.isEmpty())
		return false;

	return result;
}

void MattermostQt::FileContainer::parse_from_json(QJsonObject object)
{
	m_file_status = FileStatus::FileRemote;

	m_id = object["id"].toString("");
	m_post_id = object["post_id"].toString("");
	m_user_id = object["user_id"].toString("");
	m_mime_type = object["mime_type"].toString("");
	m_has_preview_image = object["has_preview_image"].toBool(false);
	m_name = object["name"].toString("");
	m_file_size = (qlonglong)object["size"].toDouble(0);
	m_extension = object["extension"].toString("");

	double width = object["width"].toDouble(0);
	double height = object["height"].toDouble(0);
	m_image_size.setWidth((int)width);
	m_image_size.setHeight((int)height);
	if( m_mime_type.indexOf("image/") >= 0 )
	{
		m_file_type = FileImage;
		if( m_mime_type.compare("image/gif") == 0 )
			m_file_type = FileAnimatedImage;
	}
	else
		m_file_type = FileUnknown;
}
