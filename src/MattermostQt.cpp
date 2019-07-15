#include "MattermostQt.h"

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
#include "libs/qtwebsockets/include/QtWebSockets/qwebsocket.h"
//#include <QWebSocket>

#include "MessagesModel.h"
#include "ChannelsModel.h"
#include "SettingsContainer.h"
#include "AttachedFilesModel.h"
#include "DiscountMDParser.h"

// all properties names
#define P_REPLY_TYPE         "reply_type"
#define P_API                "api"
#define P_SERVER_URL         "server_url"
#define P_SERVER_NAME        "server_name"
#define P_SERVER_INDEX       "server_index"
#define P_USER_INDEX         "user_index"
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
#define F_CONFIG_FILE       "config.json"
// some
#define cmp(s,t) s.compare(#t) == 0
#define scmp(s1,s2) s1.compare(s2) == 0
#define _compare(string) if( cmp(event,string) )

//#ifdef _RELEASE
#define request_set_headers(requset, server) \
	/*request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");*/ \
	request.setHeader(QNetworkRequest::UserAgentHeader, QString("Sailfish Mattermost v%0").arg(MATTERMOSTQT_VERSION) ); \
	if( !server->m_cookie.isEmpty() ) \
	    request.setHeader(QNetworkRequest::CookieHeader, server->m_cookie); \
	request.setRawHeader("Authorization", QString("Bearer %0").arg(server->m_token).toUtf8())
//#else
//#define request_set_headers(requset, server) \
//	request.setHeader(QNetworkRequest::ServerHeader, "application/json"); \
//	request.setHeader(QNetworkRequest::UserAgentHeader, QString("Matterfish v%0").arg(MATTERMOSTQT_VERSION) ); \
//	if( server->m_cookie.isEmpty() ) \
//	    qCritical() << "Cookie Header is missing!";\
//	else \
//	    request.setHeader(QNetworkRequest::CookieHeader, server->m_cookie); \
//	request.setRawHeader("Authorization", QString("Bearer %0").arg(server->m_token).toUtf8()); \
//	qDebug() << "Authorization " << QString("Bearer %0").arg(server->m_token)
//#endif

#define request_json(requset) \
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json")

#define request_urlencoded(requset) \
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-urlencoded")

Q_DECLARE_METATYPE(MattermostQt::FilePtr)
Q_DECLARE_METATYPE(MattermostQt::ChannelPtr)
Q_DECLARE_METATYPE(MattermostQt::MessagePtr)

MattermostQt::MattermostQt()
    : m_mdParser(nullptr)
    , m_settings(nullptr)
{
	m_networkManager.reset(new QNetworkAccessManager());

	m_update_server_timeout = 5000; // in millisecs
	m_reconnect_server.setInterval(m_update_server_timeout);
	m_user_status_timeout = 30000;  // half minute
	m_user_status_timer.setInterval(m_user_status_timeout);
	m_user_status_timer.setSingleShot(false);

	connect( &m_reconnect_server, SIGNAL(timeout()), SLOT(slot_recconect_servers()) );
	connect( &m_user_status_timer, SIGNAL(timeout()), SLOT(slot_user_status()) );

	connect(m_networkManager.data(), SIGNAL(finished(QNetworkReply*)),
	        this, SLOT(replyFinished(QNetworkReply*)));

	connect(m_networkManager.data(),SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)),
	        this, SLOT(replySSLErrors(QNetworkReply*,QList<QSslError>)));

	m_config_path = QDir(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation))
	        .filePath(QCoreApplication::applicationName());

	m_data_path = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
	m_cache_path = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);

	m_documents_path = QDir(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation))
	        .filePath(QCoreApplication::applicationName());
	m_pictures_path = QDir(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation))
	        .filePath(QCoreApplication::applicationName());
	m_download_path = QDir(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation))
	        .filePath(QCoreApplication::applicationName());

	m_settings = SettingsContainer::getInstance();
//	m_settings.reset(new SettingsContainer(this));
	connect(m_settings, SIGNAL(settingsChanged()), SLOT(slot_settingsChanged()));

	m_mdParser = new DiscountMDParser();

	load_settings();
}

MattermostQt::~MattermostQt()
{
	m_server.clear();

	delete m_mdParser;
}

QString MattermostQt::getVersion() const
{
	return MATTERMOSTQT_VERSION;
}

int MattermostQt::get_server_state(int server_index)
{
	if(server_index < 0 || server_index >= m_server.size() )
		return -1;
	return m_server[server_index]->m_state;
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

	QUrl url(server);
	url.setPath(urlString);
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
//	server->m_data_path = m_data_path + QDir::separator() +  object["server_dir"].toString("");
//	server->m_cache_path = m_cache_path + QDir::separator() +  object["server_dir"].toString("");
//	if(server->m_data_path.isEmpty())
//	{
//		server->m_data_path = m_data_path + QDir::separator() + QString("%0_%1").arg(i).arg(user_id);
//		server->m_cache_path = m_cache_path + QDir::separator() + QString("%0_%1").arg(i).arg(user_id);
//	}
	server->m_trust_cert = trustCertificate;
	server->m_display_name = display_name;
	server->m_self_index = m_server.size();
	server->m_ca_cert_path = ca_cert_path;
	server->m_cert_path = cert_path;
//	server->m_user_id = user_id;
	m_server.append(server);
	get_login(server);
	emit serverAdded(server);
}

void MattermostQt::get_login(MattermostQt::ServerPtr sc)
{
	// login by saved token

	// fix api minimum version
	if(sc->m_api <= 3)
		sc->m_api = 4;

	QString urlString = QLatin1String("/api/v")
	        + QString::number(sc->m_api)
	        + QLatin1String("/users/me");

	QUrl url(sc->m_url);
	url.setPath(urlString);
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
	url.setPath(urlString);
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
	url.setPath(urlString);
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
	url.setPath(urlString);
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
	url.setPath(urlString);
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
	// we think all indexes is right
	ServerPtr sc = m_server[server_index];
	FilePtr file = sc->m_file[file_sc_index];
	QString file_id = file->m_id;
	//files/{file_id}/thumbnail
	QString urlString = QLatin1String("/api/v")
	        + QString::number(sc->m_api)
	        + QLatin1String("/files/")
	        + file_id
	        + QLatin1String("/thumbnail");

	QUrl url(sc->m_url);
	url.setPath(urlString);
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
	// we think all indexes is right
	ServerPtr sc = m_server[server_index];
	FilePtr file = sc->m_file[file_sc_index];
	QString file_id = file->m_id;
	//files/{file_id}/preview
	QString urlString = QLatin1String("/api/v")
	        + QString::number(sc->m_api)
	        + QLatin1String("/files/")
	        + file_id
	        + QLatin1String("/preview");

	QUrl url(sc->m_url);
	url.setPath(urlString);
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
	// search in requested? and downloaded files same id
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

	// TODO first look file info in filesystem {conf_dir}/{server_dir}/files/{file_id}/file.json
	// first try search file locally
	if( f->load_json( sc->m_data_path ) )
	{
		qInfo() << "File info.json - exists! Try search file data in local storage.";
		if( f->m_file_status == FileStatus::FileDownloaded )
			return;
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
	url.setPath(urlString);
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
	ServerPtr sc = m_server[server_index];
	ChannelPtr channel;
	if( channel_type == ChannelDirect )
		channel = sc->m_direct_channels[channel_index];
	else {
		TeamPtr tc = sc->m_teams[team_index];
		if( channel_type == ChannelPrivate )
			channel = tc->m_private_channels[channel_index];
		else
			channel = tc->m_public_channels[channel_index];
	}
	MessagePtr message = channel->m_message[message_index];
	FilePtr file = message->m_file[file_index];

	//files/{file_id}/thumbnail
	QString urlString = QLatin1String("/api/v")
	        + QString::number(sc->m_api)
	        + QLatin1String("/files/")
	        + file->m_id;

	QUrl url(sc->m_url);
	url.setPath(urlString);
	QNetworkRequest request;

	request.setUrl(url);
	request_set_headers(request,sc);
	request_urlencoded(request);

	file->m_file_status = FileStatus::FileDownloading;
	emit fileStatusChanged(file->m_id, file->m_file_status);
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
	url.setPath(urlString);
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
                                     int channel_index)
{
	ChannelPtr channel = channelAt(server_index,team_index,channel_type,channel_index);
	if(!channel)
	{
		qWarning() << "Cannot find channel!!!";
		return;
	}
	ServerPtr sc = m_server[server_index];
	//files/{file_id}/thumbnail
	QString urlString = QLatin1String("/api/v")
	        + QString::number(sc->m_api)
	        + QLatin1String("/posts");

	QUrl url(sc->m_url);
	url.setPath(urlString);
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
	root["props"] = "";
	json.setObject(root);

	QNetworkReply *reply = m_networkManager->post(request, json.toJson());
	reply->setProperty(P_TRUST_CERTIFICATE, QVariant(sc->m_trust_cert) );
	reply->setProperty(P_REPLY_TYPE, QVariant(ReplyType::rt_post_send_message) );
	reply->setProperty(P_SERVER_INDEX, QVariant(server_index) );
	reply->setProperty(P_TEAM_INDEX, QVariant(team_index) );
	reply->setProperty(P_CHANNEL_INDEX, QVariant(channel_index) );
	reply->setProperty(P_CHANNEL_TYPE, QVariant((int)channel_type) );
}

void MattermostQt::delete_message(int server_index, int team_index, int channel_type, int channel_index, int message_index)
{
	ChannelPtr channel = channelAt(server_index, team_index,channel_type,channel_index);
	if(!channel)
		return;
	ServerPtr sc = m_server[server_index];
	MessagePtr message = channel->m_message[message_index];

	QString urlString = QLatin1String("/api/v")
	        + QString::number(sc->m_api)
	        + QLatin1String("/posts/")
	        + message->m_id;

	QUrl url(sc->m_url);
	url.setPath(urlString);
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
	url.setPath(urlString);
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
	reply->setProperty(P_REPLY_TYPE, QVariant(ReplyType::rt_post_message_edit) );
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
	url.setPath(urlString);
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
	url.setPath(urlString);
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
	bool direct_channel = (bool)(team_index == -1);
	if( server_index < 0 || server_index >= m_server.size() )
		return;
	ServerPtr sc = m_server[server_index];

	QString urlString = QLatin1String("/api/v")
	        + QString::number(sc->m_api)
	        + QLatin1String("/users/")
	        + userId;

	QUrl url(sc->m_url);
	url.setPath(urlString);
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
}

void MattermostQt::get_teams_unread(int server_index)
{
	if( server_index < 0 || server_index >= m_server.size() )
		return;
	get_teams_unread(m_server[server_index]);
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
	url.setPath(urlString);
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
	url.setPath(urlString);
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
	url.setPath(urlString);
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

QString MattermostQt::user_id(int server_index) const
{
	return m_server[server_index]->m_user_id;
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
	url.setPath(urlString);
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

	QDir dir(m_data_path);
	if(!dir.exists())
		dir.mkpath(m_data_path);
	QFile jsonFile( m_data_path + QDir::separator() + QLatin1String(F_CONFIG_FILE) );
	if( !jsonFile.open(QFile::WriteOnly) )
		return false;
	jsonFile.write(json.toJson());
	jsonFile.close();

	return true;
}

bool MattermostQt::load_settings()
{
	QJsonDocument json;
	QFile jsonFile( m_data_path + QDir::separator() + QLatin1String(F_CONFIG_FILE) );
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
		m_server.append(server);
		get_login(server);
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
			if(self_chat)
				sc->m_direct_channels[channel_index]->m_display_name = sc->m_user[i]->m_username
				        + QLatin1String(" ") // for right translation, if someone forgot about space before '(you)'
				        + QObject::trUtf8("(you)");
			else
				sc->m_direct_channels[channel_index]->m_display_name = sc->m_user[i]->m_username;
			sc->m_direct_channels[channel_index]->m_dc_user_index = sc->m_user[i]->m_self_index;
			QVector<int> roles;
			roles << ChannelsModel::DisplayName << ChannelsModel::AvatarPath;
			emit updateChannel(ct, roles);
			return;
		}
	}
	// send request for user credentials first
	get_user_info(sc->m_self_index, user_id);
}

void MattermostQt::prepare_user_index(int server_index, MattermostQt::MessagePtr message)
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
		get_user_info(server_index, message->m_user_id);
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

MattermostQt::ChannelPtr MattermostQt::channelAt(int server_index, int team_index, int channel_type, int channel_index)
{
	ChannelPtr channel;
	if( server_index < 0 || server_index >= m_server.size() )
		return channel;
	if(channel_index < 0)
		return channel;
	ServerPtr sc = m_server[server_index];
	if(channel_type == ChannelType::ChannelDirect)
	{
		if(channel_index >= sc->m_direct_channels.size() )
			return channel;
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
			return channel;

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



void MattermostQt::setSettingsContainer(SettingsContainer *settings)
{
	m_settings = settings;
}

SettingsContainer *MattermostQt::settings()
{
	return m_settings;
}

void MattermostQt::websocket_connect(ServerPtr server)
{
	// server get us authentificztion token, time to open WebSocket!
	QSharedPointer<QWebSocket> socket(new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this));
	server->m_socket = socket;
	socket->setProperty(P_SERVER_INDEX,server->m_self_index);

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

	QString urlString = QLatin1String("/api/v")
	        + QString::number(server->m_api)
	        + QLatin1String("/websocket");

	QString serUrl = server->m_url;
	serUrl.replace("https://","wss://")
	        .replace("http://","ws://");
	QUrl url(serUrl);
	url.setPath(urlString);

//	QNetworkRequest request;
//	request_set_headers(request,server);
//	request.setUrl(url);

	socket->open(url);
}

bool MattermostQt::reply_login(QNetworkReply *reply)
{
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
			//qDebug() << object;
			server->m_user_id = object["id"].toString();
		}
		QString server_dir = QString("%0_%1").arg(server->m_self_index).arg(server->m_user_id);
		if( server->m_data_path.isEmpty() )
			server->m_data_path = m_data_path + QDir::separator() +  server_dir;
		if( server->m_cache_path.isEmpty() )
			server->m_cache_path = m_cache_path + QDir::separator() +  server_dir;
		websocket_connect(server);
		// TODO add singnal server Added
//		emit serverConnected(server->m_self_index);
		return true;
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
			server->m_trust_cert = reply->property(P_TRUST_CERTIFICATE).toBool();
			server->m_ca_cert_path = reply->property(P_CA_CERT_PATH).toString();
			server->m_cert_path = reply->property(P_CERT_PATH).toString();
			server->m_cert  = reply->sslConfiguration();
			server->m_cookie= reply->header(QNetworkRequest::CookieHeader).toString();
			server->m_self_index =  m_server.size();

			QJsonDocument json = QJsonDocument::fromJson( reply->readAll() );
			if( json.isObject() )
			{
				QJsonObject object = json.object();
				//qDebug() << object;
				server->m_user_id = object["id"].toString();
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
						m_server[i]->m_cookie = server->m_cookie;
						//server.reset();
						server = m_server[i];
						break;
					}
				}
			}

			if(is_new_account)
				m_server.append( server );
			//get_login(server);
			websocket_connect(server);

			if(is_new_account)
				emit serverAdded(server);

			return true;
		}
	}
	return false;
}

void MattermostQt::reply_get_teams(QNetworkReply *reply)
{
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
				m_server[server_index]->m_teams.append(tc);
				// TODO move to Thread all filesystem operations
				tc->save_json( m_server[server_index]->m_data_path );
				// ---------------------------------------------
				emit teamAdded(tc);
			}
			else
				qDebug() << "array[" << i << "]: " << array.at(i);
		}
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
	Q_UNUSED(server_index)
	if(!is_ok)
		return;
	QJsonDocument json = QJsonDocument::fromJson(reply->readAll());
	QJsonArray array = json.array();

	if( array.isEmpty() )
	{
		qWarning() << "Wrong Json" << json;
		return;
	}

//	qDebug() << reply->header(QNetworkRequest::LastModifiedHeader);

	for( int i = 0; i < array.size(); i++ )
	{
		QJsonObject object = array.at(i).toObject();
		if(object.isEmpty())
			continue;
		QString team_id = object["team_id"].toString();
		qlonglong msg_count = (qlonglong)object["mgg_count"].toDouble();
		qlonglong mention_count = (qlonglong)object["mention_count"].toDouble();
		emit teamUnread(team_id, (int)msg_count, (int)mention_count);
	}
}

void MattermostQt::reply_get_posts(QNetworkReply *reply)
{
	int server_index = reply->property(P_SERVER_INDEX).toInt();
	int team_index = reply->property(P_TEAM_INDEX).toInt();
	int channel_index = reply->property(P_CHANNEL_INDEX).toInt();
	int channel_type =reply-> property(P_CHANNEL_TYPE).toInt();

	ChannelPtr channel = channelAt(server_index,team_index,channel_type,channel_index);
	if(!channel)
	{
		qWarning() << "Chanel not found!";
		return;
	}
//	if(channel->m_type == ChannelDirect)
//		team_index = -1;

	ServerPtr sc = m_server[server_index];
	QJsonDocument json = QJsonDocument::fromJson(reply->readAll());
	QJsonArray order = json.object()["order"].toArray();
	channel->m_message.reserve(order.size());
	QJsonObject posts = json.object()["posts"].toObject();
	QJsonObject::iterator it = posts.begin(),
	        end = posts.end();
	bool new_messages = false;
	int its_all = order.size();
	for(; it != end; it++ )
	{
		MessagePtr message(new MessageContainer(it.value().toObject()));
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
		new_messages = true;
	}
	if(new_messages)
	{
		// sorting with right order
		for(int i = 0, j2 = channel->m_message.size() - 1; i < order.size(); i++, j2 = channel->m_message.size() - 1 - i)
		{
			QString id = order[i].toString();
			for(int j = channel->m_message.size() - i - 1; j >= 0  ; j--)
			{
				if( channel->m_message[j]->m_id.compare(id) == 0 )
				{
					MessagePtr temp;
					temp = channel->m_message[j];
					channel->m_message[j] = channel->m_message[j2];
					channel->m_message[j2] = temp;
					channel->m_message[j2]->m_self_index = j2;
					channel->m_message[j]->m_self_index = j;
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
					// get user_index for post
					prepare_user_index(sc->m_self_index, temp);
					break;
				}
			}
		}
		emit messagesAdded(channel);
		if(its_all < 60 )
			emit messagesIsEnd(channel);
	}
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
		qWarning() << "Chanel not found!";
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
	for(; it != end; it++ )
	{
		MessagePtr message(new MessageContainer(it.value().toObject()));
		message->m_server_index = server_index;
		if(channel_type != ChannelDirect)
			message->m_team_index = team_index;
		else
			message->m_team_index = -1;
		message->m_channel_id = channel->m_id;
		message->m_channel_type = channel->m_type;
		message->m_channel_index = channel->m_self_index;
//		message->m_self_index = channel->m_message.size();
		messages.append(message);
		if(message->m_type == MessageOwner::MessageTypeCount)
		{
			if(message->m_user_id.compare(sc->m_user_id) == 0 )
				message->m_type = MessageMine;
			else
				message->m_type = MessageOther;
		}
		prepare_user_index(server_index,message);

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
				break;
			}
		}
	}
	int size = messages.size();
	// now add new messages to front, and change all indexes after new
	for(int i = 0; i < channel->m_message.size(); i++)
		channel->m_message[i]->m_self_index += size;
	messages.append(channel->m_message);
	channel->m_message.swap(messages);
	// get files info from new messages
	// TODO its no need more? becuse it should sck file info from MessagesModel when it need
	for(int i = 0; i < size; i++)
	{
		MessagePtr temp = channel->m_message[i];
#ifdef PRELOAD_FILE_INFOS
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
#endif
	}
	emit messagesAddedBefore(channel, size);

}

void MattermostQt::reply_get_public_channels(QNetworkReply *reply)
{
	int server_index = reply->property(P_SERVER_INDEX).toInt();
	if( server_index < 0 || server_index >= m_server.size() )
	{
		qWarning() << "Wrong server index";
		return;
	}
	ServerPtr sc = m_server[server_index];
	TeamPtr tc;
//"id": "string",
//"create_at": 0,
//"update_at": 0,
//"delete_at": 0,
//"display_name": "string",
//"name": "string",
//"description": "string",
//"email": "string",
//"type": "string",
//"allowed_domains": "string",
//"invite_id": "string",
//"allow_open_invite": true
	QJsonDocument json = QJsonDocument::fromJson(reply->readAll());
	if(json.isArray())
	{
		QJsonArray array = json.array();
		for(int i = 0; i < array.size(); i++ )
		{
			if(array.at(i).isObject())
			{
				QJsonObject object = array.at(i).toObject();
				ChannelPtr ct( new ChannelContainer(object) );

				// TODO not much secure, need test all parameters
				ct->m_server_index = server_index;
				qDebug() << QString("channel \"%0\" : %1").arg(ct->m_display_name).arg(ct->m_id);

				switch( ct->m_type )
				{
				// open channels
				case MattermostQt::ChannelPublic:
					ct->m_team_index = reply->property(P_TEAM_INDEX).toInt();
					tc.reset();
					tc = sc->m_teams[ct->m_team_index];
					ct->m_self_index = tc->m_public_channels.size();
					tc->m_public_channels.append(ct);
					emit channelAdded(ct);
					break;
				// private channels
				case MattermostQt::ChannelPrivate:
					ct->m_team_index = reply->property(P_TEAM_INDEX).toInt();
					tc.reset();
					tc = sc->m_teams[ct->m_team_index];
					ct->m_self_index = tc->m_private_channels.size();
					tc->m_private_channels.append(ct);
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
						emit channelAdded(ct);
						prepare_direct_channel(ct->m_server_index, ct->m_self_index);
					}
					break;
				default:
					break;
				}
			}
		}
	}
	post_users_status(server_index);
}

void MattermostQt::reply_get_channel(QNetworkReply *reply)
{
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
}

void MattermostQt::reply_get_user_info(QNetworkReply *reply)
{
	int server_index = reply->property(P_SERVER_INDEX).toInt();
	int team_index = reply->property(P_TEAM_INDEX).toInt();
	bool direct_channel = reply->property(P_DIRECT_CHANNEL).toBool();

	if( server_index < 0 || server_index >= m_server.size() )
	{
		qWarning() << "Error! Wrong server index";
		return;
	}
	ServerPtr sc = m_server[server_index];

	QJsonDocument json = QJsonDocument::fromJson(reply->readAll());

	qDebug() << json;

	if( !json.isObject() )
	{
		qWarning() << "Cant parse Json";
		return;
	}

	UserPtr user( new UserContainer(json.object()) );

//	for(int)
	// maby need check if user laready exists in list
	bool user_exists = false;
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
		while( it != sc->m_nouser_messages.end() )
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
				QVector<int> roles;
				roles << ChannelsModel::DisplayName << ChannelsModel::AvatarPath;
				emit updateChannel(channel, roles);
				qDebug() << QString("direct_channel \"%0\" id: '%1'").arg(channel->m_display_name).arg(channel->m_id) ;
				break;
			}
			else if( ch_name == user->m_id  )
			{
				channel->m_display_name = user->m_username;
				channel->m_dc_user_index = user->m_self_index;
				QVector<int> roles;
				roles << ChannelsModel::DisplayName << ChannelsModel::AvatarPath;
				emit updateChannel(channel, roles);
				qDebug() << QString("direct_channel \"%0\" id: '%1'").arg(channel->m_display_name).arg(channel->m_id) ;
				break;
			}
		}
		if(user->m_image_path.isEmpty() && user_exists)
			get_user_image(server_index,user->m_self_index);
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
	QVector<int> roles;
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
			if( error_id.compare("api.user.check_user_password.invalid.app_error") == 0 )
			{
				emit onConnectionError(ConnectionError::WrongPassword, QObject::trUtf8("Login failed because of invalid password"), -1 );
			}
			else if( error_id.compare("api.context.session_expired.app_error") == 0 )
			{
				int server_index  = reply->property(P_SERVER_INDEX).toInt();
				emit onConnectionError(ConnectionError::SessionExpired, message, server_index);
			}
			else
				qWarning() << json;
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
		QVector<int> file_update_roles;
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
		QVector<int> file_update_roles;
		file_update_roles << AttachedFilesModel::FilePreviewPath;
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
		qCritical() << "Something went wrong! File index is emty in reply_get_file_info()!";
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
	QVector<int> file_update_roles;

	file_update_roles << AttachedFilesModel::FileType;
	file_update_roles << AttachedFilesModel::FileName;
	file_update_roles << AttachedFilesModel::FileSize;
	file_update_roles << AttachedFilesModel::FileMimeType;
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
	// TODO Load_json happens in get_file_info request prepare function, no need it here
	/*if( file->load_json(sc->m_data_path) )
	{
		if(file->m_file_type == FileImage || file->m_file_type == FileAnimatedImage )
		{
			if( file->m_thumb_path.isEmpty() || !QFile::exists(file->m_thumb_path) )
				get_file_thumbnail(server_index,file->m_self_sc_index);
			bool download_current_file = true;
			if( m_settings && file->m_file_size > m_settings->autoDownloadImageSize())
				download_current_file = false;
			if( file->m_has_preview_image && !download_current_file
			        && !QFile::exists(file->m_preview_path) )
			{
				file->m_preview_path.clear();
				get_file_preview(server_index,file->m_self_sc_index);
				file->m_file_status = FileRemote;
			}
			if( file->m_file_path.isEmpty() || !QFile::exists(file->m_file_path) )
			{
				file->m_file_path.clear();
				if( download_current_file )
				{
					file->m_file_status = FileDownloading;
					get_file(file->m_server_index, file->m_team_index,
					         file->m_channel_type, file->m_channel_index,
					         file->m_message_index, file->m_self_index);
				}
				else
					file->m_file_status = FileRemote;
			}
			else
				file->m_file_status = FileDownloaded;
		}
		file_update_roles << AttachedFilesModel::FileStatus;
		isUpdateMessage = true;
	}
	else//*/
	if(file->m_file_type == FileImage || file->m_file_type == FileAnimatedImage )
	{
//		QString file_thumb_path = sc->m_cache_path + QString("/files/%0/thumb.jpeg").arg(file->m_id);
		if( file->m_thumb_path.isEmpty() )
			file->m_thumb_path = sc->m_cache_path + QString("/files/%0/thumb.jpeg").arg(file->m_id);
		if( !QFile::exists(file->m_thumb_path) ) {
			file->m_thumb_path.clear();
			get_file_thumbnail(server_index,file->m_self_sc_index);
		}
		else {
			file_update_roles << AttachedFilesModel::FileThumbnailPath;
			isUpdateMessage = true;
		}
		bool download_current_file = true;
		if( m_settings && file->m_file_size > m_settings->autoDownloadImageSize())
			download_current_file = false;

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
		else
		{
			// TODO - download files to cahce? while user not request "save to gallery" or "download"
			if(file->m_file_path.isEmpty())
				file->m_file_path = m_documents_path + QDir::separator() + file->filename();
			if( !QFile::exists(file->m_file_path) ) {
				file->m_file_path.clear();
				get_file(file->m_server_index, file->m_team_index,
				         file->m_channel_type, file->m_channel_index,
				         file->m_message_index, file->m_self_index);
			}
			else {
				file_update_roles << AttachedFilesModel::FilePath;
				isUpdateMessage = true;
			}
		}
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

	QVector<int> file_update_roles;
	file_update_roles << AttachedFilesModel::FileStatus;
	emit attachedFilesChanged(mc, QVector<QString>(), file_update_roles);
}

void MattermostQt::reply_get_file(QNetworkReply *reply)
{
	FilePtr file =  reply->property(P_FILE_PTR).value<FilePtr>();
//	if(file)
//		qDebug() << file->m_name;
	QString dowload_dir;
	if(file->m_file_type == FileImage || file->m_file_type == FileAnimatedImage)
		dowload_dir = m_pictures_path;
	else
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
			QVector<int> roles;
			roles << AttachedFilesModel::FileStatus;
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
				QVector<int> roles;
				roles << ChannelsModel::AvatarPath;
				emit updateChannel(c, roles);
				break;
			}
		}
		QVector<int> roles;
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

void MattermostQt::reply_post_message_edit(QNetworkReply *reply)
{
	qDebug() << reply->readAll();
}

void MattermostQt::event_posted(ServerPtr sc, QJsonObject data)
{
	ChannelType type = ChannelType::ChannelTypeCount;
	QString ch_type = data["channel_type"].toString();

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
	MessagePtr message( new MessageContainer(post) );

//	message_/format(message);

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
			if( message->m_type != MessageMine )
			// that for notifications
				emit newMessage(message);
		}
	}
	else
	{
		QString team_id = data["team_id"].toString();
		QString channel_id = message->m_channel_id;
		ChannelPtr channel;
		int channel_index = -1;

		for(int i = 0; i < sc->m_teams.size(); i++ )
		{
			if( scmp(sc->m_teams[i]->m_id,team_id) )
			{
				TeamPtr tc = sc->m_teams[i];
				QVector<ChannelPtr> *channels;
				if( type == ChannelType::ChannelPublic )
					channels = &tc->m_public_channels;
				else if( type == ChannelType::ChannelPrivate )
					channels = &tc->m_private_channels;
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
						team_index = i;
						break;
					}
				}
				break;
			}
		}
		if( channel && channel_index >= 0)
		{
			// TODO refactoring, move all creation parameters to constructor
			message->m_channel_type  = channel->m_type;
			message->m_team_index    = team_index;
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
				emit newMessage(message);
		}
	}
	if(message->m_channel_index == -1 && message->m_team_index == -1)
	{// Need some structure for unnatached messages (when it need)
	//maybe need use only channels list in ServerPtr,
	//and download TeamPtr and teamslist after it needed by user
//		UMessagePtr umessage(new UnattachedMessageContainer);
//		umessage->m_team_id = data["team_id"].toString();
//		umessage->m_message = message;
//		sc->m_untacched_messages.append(umessage);
		if( message->m_type != MessageMine )
			emit newMessage(message);
	}

	// TODO try formating message
}

void MattermostQt::event_post_edited(MattermostQt::ServerPtr sc, QJsonObject object)
{
	QJsonObject data = object["data"].toObject();
	QJsonObject broadcast = object["broadcast"].toObject();
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
	MessagePtr message( new MessageContainer(post) );

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

void MattermostQt::event_status_change(MattermostQt::ServerPtr sc, QJsonObject data)
{
	//qDebug() << data;
	//{"status":"online","user_id":"gqr15ebytjg7znhh4boz74foxy"}
	UserStatus status = str2status(data["status"].toString());
	UserPtr current = id2user(sc,data["user_id"].toString());
	if(!current)
	{// TODO try to send request for user (if it not found)
		// and mark this request (that requested from here)
		return; // FIXME
	}
	current->m_status = status;
	QVector<int> roles;
	roles << UserStatusRole;
	emit userUpdated(current, roles);
}

void MattermostQt::event_typing(MattermostQt::ServerPtr sc, QJsonObject data)
{
	qDebug() << data;
	//{"status":"online","user_id":"gqr15ebytjg7znhh4boz74foxy"}
	//UserStatus status = str2status(data["status"].toString());
	//UserPtr current = id2user(sc,data["user_id"].toString());
	//if(!current)
	//{// TODO try to send request for user (if it not found)
	    // and mark this request (that requested from here)
	    return; // FIXME
	//}
	//current->m_status = status;
	//QVector<int> roles;
	//roles << UserStatusRole;
	//emit userUpdated(current, roles);
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
	for(int i = 0; sc->m_user.size(); i++)
	{
		if( sc->m_user[i]->m_id == id )
			return sc->m_user[i];
	}
	return UserPtr();
}

//void MattermostQt::message_format(MattermostQt::MessagePtr message)
//{
//	if(message.isNull() || message->m_message.isEmpty())
//		return;

//	QString html;
//	QString &m = message->m_message;
//	QStringList tokens;
//	tokens << "\\*.*\\*";
//	for(int i = 0; i < message->m_message.size(); i++)
//	{

//	}
////	QRegExp md("(```|\\*{1,3}(.*^\\*)\\*{1,3}|\^#+)\n");
////	int last_pos = 0;
////	while( md.indexIn(m,last_pos) != -1 )
////	{
////		if(last_pos = 0)
////			html += m.left(cap)
////	}
//}

void MattermostQt::event_post_deleted(MattermostQt::ServerPtr sc, QJsonObject data)
{
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
	MessagePtr message( new MessageContainer(post) );

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
//	qDebug() << post; //search channel
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
				channel->m_message.remove(i);
				break;
			}
		}
		if(deleted && index >= 0)
		{
			for(int i = index; i < channel->m_message.size(); i++ )
				channel->m_message[i]->m_self_index = i;
			emit messageDeleted(deleted);
		}
		return;
	}
}

void MattermostQt::replyFinished(QNetworkReply *reply)
{
	QVariant replyType;
	replyType = reply->property(P_REPLY_TYPE);
	if (reply->error() == QNetworkReply::NoError) {
		//success
		if(reply->header(QNetworkRequest::LastModifiedHeader).isValid())
			qDebug() << "LastModified" << reply->header(QNetworkRequest::LastModifiedHeader);

		if(replyType.isValid())
		{
			switch (replyType.toInt()) {
			case ReplyType::rt_login:
				if( reply_login(reply) )
				{//connect timers
//					connect( &m_update_server, SIGNAL(timeout()), SLOT(slot_get_teams_unread()) );
//					m_update_server.setTimerType(Qt::/*TimerType*/);
//					m_update_server.start();
//					slot_get_teams_unread();
				}
				break;
			case ReplyType::rt_get_teams:
				reply_get_teams(reply);
				break;
			case ReplyType::rt_get_public_channels:
				reply_get_public_channels(reply);
				break;
			case ReplyType::rt_get_channel:
				reply_get_channel(reply);
				break;
			case ReplyType::rt_post_channel_view:
				reply_post_channel_view(reply);
				break;
			case ReplyType::rt_get_user_info:
				reply_get_user_info(reply);
				break;
			case ReplyType::rt_post_users_status:
				reply_post_users_status(reply);
				break;
			case ReplyType::rt_get_user_image:
				reply_get_user_image(reply);
				break;
			case ReplyType::rt_get_team:
				reply_get_team(reply);
				break;
			case ReplyType::rt_get_teams_unread:
				reply_get_teams_unread(reply);
				break;
			case ReplyType::rt_get_posts:
				reply_get_posts(reply);
				break;
			case ReplyType::rt_get_posts_before:
				reply_get_posts_before(reply);
				break;
			case ReplyType::rt_get_file_thumbnail:
				reply_get_file_thumbnail(reply);
				break;
			case ReplyType::rt_get_file_preview:
				reply_get_file_preview(reply);
				break;
			case ReplyType::rt_get_file_info:
				reply_get_file_info(reply);
				break;
			case ReplyType::rt_get_file:
				reply_get_file(reply);
				break;
			case ReplyType::rt_post_file_upload:
				reply_post_file_upload(reply);
				break;
			case ReplyType::rt_post_send_message:
				reply_post_send_message(reply);
				break;
			case ReplyType::rt_delete_message:
				reply_delete_message(reply);
				break;
			case ReplyType::rt_post_message_edit:
				reply_post_message_edit(reply);
				break;
			default:
				qWarning() << "That can't be!";
				qDebug() << "Reply:" << QString::fromUtf8( reply->readAll() );

				QList<QByteArray> headers = reply->rawHeaderList();
				foreach(QByteArray header, headers)
				{
					qDebug() << header;
				}
				break;
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
		qDebug() << "Failure: " << reply->error() << reply->errorString();
		qDebug() << reply;
//		qDebug() << "Reply: " << reply->readAll();
		reply_error(reply);
		if( reply->error() == QNetworkReply::AuthenticationRequiredError)
		{// need authentification
			// TODO show error in LoginPage about bad authetificaation
			//qWarning() << reply->;
		}
		else
		{
			for(int i = 0; i < server().size(); i ++ )
			{
				if(server().at(i)->m_socket)
					server().at(i)->m_socket->ping(QString("ping").toUtf8());
			}
		}

		if(replyType.isValid())
		{
			switch(replyType.toInt())
			{
			case ReplyType::rt_get_file_info:

				break;
			}
		}
	}
	delete reply;
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
	emit fileDownloadingProgress(file->m_id, (qreal)bytesReceived/bytesTotal);
}

void MattermostQt::replyUploadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
	QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
	if(!reply)
		return;
	ChannelPtr channel = reply->property(P_CHANNEL_PTR).value<ChannelPtr>();
	QString data;
	if(channel)
		data = channel->m_id;
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
//	emit serverConnected(server_index);
}

void MattermostQt::onWebSocketSslError(QList<QSslError> errors)
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
	QWebSocket *socket = qobject_cast<QWebSocket*>(sender());
	if(socket)
		socket->ignoreSslErrors(ignoreErrors);
//	qDebug() << err;
}

void MattermostQt::onWebSocketError(QAbstractSocket::SocketError error)
{
	qWarning() << error;
	QWebSocket * socket = qobject_cast<QWebSocket*>(sender());
	if(!socket) // strange situation, if it happens
		return;

	qWarning() << socket->errorString();
}

void MattermostQt::onWebSocketStateChanged(QAbstractSocket::SocketState state)
{
	qDebug() << state;
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
	switch(state) {
	case QAbstractSocket::UnconnectedState:
		m_reconnect_server.start();
		sc->m_state = (int)state;
		break;
	case QAbstractSocket::ConnectingState:
		sc->m_state = (int)state;
		break;
	case QAbstractSocket::ConnectedState:
		{
			sc->m_state = (int)state;
			bool need_reconnect = false;
			for(int i = 0; i < m_server.size(); i++ )
			{
				if( m_server[i]->m_state != ServerConnected )
					need_reconnect = true;
			}
			if(!need_reconnect)
				m_reconnect_server.stop();
			emit serverStateChanged(server_index, (int)state);
		}
		break;
	case QAbstractSocket::HostLookupState:
		break;
	case QAbstractSocket::BoundState:
		break;
	case QAbstractSocket::ListeningState:
		break;
	case QAbstractSocket::ClosingState:
		break;
	}
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
	QJsonObject data = object["data"].toObject();
	QJsonObject broadcast = object["broadcast"].toObject();

	_compare(hello) // that mean we are logged in
	{
		//qDebug() << event;
		save_settings();
		//m_user_status_timer.setSingleShot(false);
		m_user_status_timer.start();
		emit serverConnected(sc->m_self_index);
	}
	else _compare(posted)
	    event_posted(sc,data);
	else _compare(post_edited)
	    event_post_edited(sc,object);
	else _compare(post_deleted)
	    event_post_deleted(sc,data);
	else _compare(status_change)
	    event_status_change(sc,data);
	else _compare(typing)
	    event_typing(sc,object);
	else
	    qWarning() << message;
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
//channel_viewed
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
		if( m_server[i]->m_state == ServerUnconnected  && m_server[i]->m_socket)
		{
			QString urlString = QLatin1String("/api/v")
			        + QString::number(m_server[i]->m_api)
			        + QLatin1String("/websocket");

			QString serUrl = m_server[i]->m_url;
			serUrl.replace("https://","wss://")
			        .replace("http://","ws://");
			QUrl url(serUrl);
			url.setPath(urlString);

//			QNetworkRequest request;
//			request_set_headers(request,m_server[i]);
//			request.setUrl(url);

			m_server[i]->m_socket->open(url);
		}
	}
}

void MattermostQt::slot_user_status()
{
	for(int i = 0; i < m_server.size(); i ++ )
	{
		post_users_status(i);
	}
}

void MattermostQt::slot_settingsChanged()
{
	// TODO save to settings file
	save_settings();
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

MattermostQt::ServerContainer::~ServerContainer()
{
	if( m_socket ){
		m_socket->blockSignals(true);
		m_socket->close(QWebSocketProtocol::CloseCodeGoingAway, QString("Client closing") );
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

MattermostQt::MessageContainer::MessageContainer(QJsonObject object)
{
	m_user_index = -1;
	m_server_index = -1;
	m_channel_index  = -1;
	m_team_index = -1;
//	qDebug() << object;
	m_id = object["id"].toString();
	m_channel_id = object["channel_id"].toString();
	m_message = object["message"].toString();
	m_type_string = object["type"].toString();
	m_user_id = object["user_id"].toString();
	m_create_at = (qlonglong)object["create_at"].toDouble(0);
	m_update_at = (qlonglong)object["update_at"].toDouble(0);
	m_delete_at = (qlonglong)object["delete_at"].toDouble(0);
	if( m_type_string.indexOf("system_") >= 0 )
		m_type = MessageOwner::MessageSystem;
	else
		m_type = MessageOwner::MessageTypeCount;
	QJsonArray filenames = object["filenames"].toArray();
	QJsonArray file_ids = object["file_ids"].toArray();
	for(int i = 0; i < filenames.size(); i++ )
		m_filenames.append( filenames.at(i).toString() );
	for(int i = 0; i < file_ids.size(); i++ )
		m_file_ids.append( file_ids.at(i).toString() );
}

MattermostQt::FilePtr MattermostQt::MessageContainer::fileAt(int file_index)
{
	if( file_index < 0 || file_index >= m_file.size() )
		return MattermostQt::FilePtr();
	return m_file[file_index];
}

MattermostQt::FileContainer::FileContainer(QJsonObject object) noexcept
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
	QJsonObject item_size;
	item_size["width"] = m_item_size.width();
	item_size["height"] = m_item_size.height();
	obj["item_size"] = item_size;
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
//	m_item_size.setWidth(item_size["width"].toDouble());
//	m_item_size.setHeight(item_size["height"].toDouble());
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
	if( m_file_type == FileType::FileImage )
	{
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
