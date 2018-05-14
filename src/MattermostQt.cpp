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

//#include "libs/qtwebsockets/include/QtWebSockets/qwebsocket.h"
//#include <QtWebSockets>

#define P_REPLY_TYPE         "reply_type"
#define P_API                "api"
#define P_SERVER_URL         "server_url"
#define P_SERVER_NAME        "server_name"
#define P_SERVER_INDEX       "server_index"
#define P_USER_INDEX         "user_index"
#define P_FILE_SC_INDEX      "file_sc_index"
#define P_TEAM_INDEX         "team_index"
#define P_MESSAGE_INDEX      "message_index"
#define P_TEAM_ID            "team_id"
#define P_CHANNEL_INDEX      "channel_id"
#define P_CHANNEL_TYPE       "channel_type"
#define P_TRUST_CERTIFICATE  "trust_certificate"
#define P_DIRECT_CHANNEL     "direct_channel"
#define P_CA_CERT_PATH       "ca_cert_path"
#define P_CERT_PATH          "cert_path"
#define P_NEED_SAVE_SETTINGS "save_settings"

#define F_CONFIG_FILE       "config.json"

#define cmp(s,t) s.compare(#t) == 0
#define scmp(s1,s2) s1.compare(s2) == 0
#define _compare(string) if( cmp(event,string) )

#define requset_set_headers(requset, server) \
	request.setHeader(QNetworkRequest::ServerHeader, "application/json"); \
	request.setHeader(QNetworkRequest::UserAgentHeader, QString("MattermosQt v%0").arg(MATTERMOSTQT_VERSION) ); \
	request.setHeader(QNetworkRequest::CookieHeader, server->m_cookie); \
	request.setRawHeader("Authorization", QString("Bearer %0").arg(server->m_token).toUtf8())

MattermostQt::MattermostQt()
{
	m_networkManager.reset(new QNetworkAccessManager());

	m_update_server_timeout = 5000; // in millisecs
	m_reconnect_server.setInterval(m_update_server_timeout);
	connect( &m_reconnect_server, SIGNAL(timeout()), SLOT(slot_recconect_servers()) );

	connect(m_networkManager.data(), SIGNAL(finished(QNetworkReply*)),
	        this, SLOT(replyFinished(QNetworkReply*)));

	connect(m_networkManager.data(),SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)),
	        this, SLOT(replySSLErrors(QNetworkReply*,QList<QSslError>)));

	m_settings_path = QStandardPaths::displayName(QStandardPaths::AppDataLocation)
	        + QLatin1String("/mattermostqt/");
	m_settings_path = "/home/nemo/.config/mattermostqt/";
	load_settings();
}

MattermostQt::~MattermostQt()
{
	m_server.clear();
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
	reply->setProperty(P_REPLY_TYPE, QVariant(ReplyType::Login) );
	reply->setProperty(P_API, QVariant(api) );
	reply->setProperty(P_SERVER_URL, server );
	reply->setProperty(P_SERVER_NAME, display_name );
	reply->setProperty(P_CA_CERT_PATH, ca_cert_path );
	reply->setProperty(P_CERT_PATH, cert_path );
	reply->setProperty(P_TRUST_CERTIFICATE, trustCertificate );

//	// Load previosly saved certificate
	if( trustCertificate )
	{
		QFile ca_cert_file(ca_cert_path);
		QFile cert_file(cert_path);
		ca_cert_file.open(QIODevice::ReadOnly);
		cert_file.open(QIODevice::ReadOnly);
		QSslCertificate ca_cert(&ca_cert_file, QSsl::Pem);
		QSslCertificate cert(&cert_file, QSsl::Pem);
		QList<QSslError> errors;
		errors << QSslError(QSslError::CertificateUntrusted, ca_cert);
		errors << QSslError(QSslError::SelfSignedCertificateInChain, ca_cert);
		errors << QSslError(QSslError::SelfSignedCertificate, ca_cert);
		errors << QSslError(QSslError::CertificateUntrusted, cert);
		errors << QSslError(QSslError::SelfSignedCertificateInChain, cert);
		errors << QSslError(QSslError::SelfSignedCertificate, cert);
		reply->ignoreSslErrors(errors);

		ca_cert_file.close();
		cert_file.close();
	}
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
	requset_set_headers(request,sc);

	QNetworkReply *reply = m_networkManager->get(request);
	reply->setProperty(P_REPLY_TYPE, QVariant(ReplyType::Login) );
	reply->setProperty(P_SERVER_INDEX, sc->m_self_index);
	reply->setProperty(P_TRUST_CERTIFICATE, sc->m_trust_cert);

	// Load previosly saved certificate
	if( sc->m_trust_cert )
	{
		QFile ca_cert_file(sc->m_ca_cert_path);
		QFile cert_file(sc->m_cert_path);
		ca_cert_file.open(QIODevice::ReadOnly);
		cert_file.open(QIODevice::ReadOnly);
		QSslCertificate ca_cert(&ca_cert_file, QSsl::Pem);
		QSslCertificate cert(&cert_file, QSsl::Pem);
		QList<QSslError> errors;
		errors << QSslError(QSslError::CertificateUntrusted, ca_cert);
		errors << QSslError(QSslError::SelfSignedCertificateInChain, ca_cert);
		errors << QSslError(QSslError::SelfSignedCertificate, ca_cert);
		errors << QSslError(QSslError::CertificateUntrusted, cert);
		errors << QSslError(QSslError::SelfSignedCertificateInChain, cert);
		errors << QSslError(QSslError::SelfSignedCertificate, cert);
		reply->ignoreSslErrors(errors);
		ca_cert_file.close();
		cert_file.close();
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
	        + QLatin1String("/users/me/teams");

	QUrl url(sc->m_url);
	url.setPath(urlString);
	QNetworkRequest request;
	QJsonDocument json;
	QJsonObject data;

	//json.setObject(data)

	request.setUrl(url);
	requset_set_headers(request,sc);

	if(sc->m_trust_cert)
		request.setSslConfiguration(sc->m_cert);

	QNetworkReply *reply = m_networkManager->get(request);
	reply->setProperty(P_REPLY_TYPE, QVariant(ReplyType::Teams) );
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
	requset_set_headers(request,sc);
//	if(sc->m_trustCertificate && !sc->m_cert.isNull())
//		request.setSslConfiguration(sc->m_cert);

	QNetworkReply *reply = m_networkManager->get(request);
	reply->setProperty(P_REPLY_TYPE, QVariant(ReplyType::Channels) );
	reply->setProperty(P_SERVER_INDEX, QVariant(server_index) );
	reply->setProperty(P_TEAM_INDEX, QVariant(team_index) );
	reply->setProperty(P_TEAM_ID, QVariant(team_id) );
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
	requset_set_headers(request,sc);
//	if(sc->m_trustCertificate && !sc->m_cert.isNull())
//		request.setSslConfiguration(sc->m_cert);

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
	requset_set_headers(request,sc);

	QNetworkReply *reply = m_networkManager->get(request);
	reply->setProperty(P_TRUST_CERTIFICATE, QVariant(sc->m_trust_cert) );
	reply->setProperty(P_REPLY_TYPE, QVariant(ReplyType::rt_get_file_thumbnail) );
	reply->setProperty(P_SERVER_INDEX, QVariant(server_index) );
	reply->setProperty(P_FILE_SC_INDEX, QVariant(file_sc_index) );
}

void MattermostQt::get_file_info(int server_index, int team_index, int channel_type,
                                 int channel_index, int message_index, QString file_id)
{// TODO first look file info in filesystem {conf_dir}/{server_dir}/files/{file_id}/file.json
	// we think all indexes is right
	ServerPtr sc = m_server[server_index];
	//files/{file_id}/thumbnail
	QString urlString = QLatin1String("/api/v")
	        + QString::number(sc->m_api)
	        + QLatin1String("/files/")
	        + file_id
	        + QLatin1String("/info");

	QUrl url(sc->m_url);
	url.setPath(urlString);
	QNetworkRequest request;

	request.setUrl(url);
	requset_set_headers(request,sc);

	QNetworkReply *reply = m_networkManager->get(request);
	reply->setProperty(P_TRUST_CERTIFICATE, QVariant(sc->m_trust_cert) );
	reply->setProperty(P_REPLY_TYPE, QVariant(ReplyType::rt_get_file_info) );
	reply->setProperty(P_SERVER_INDEX, QVariant(server_index) );
	reply->setProperty(P_TEAM_INDEX, QVariant(team_index) );
	reply->setProperty(P_CHANNEL_INDEX, QVariant(channel_index) );
	reply->setProperty(P_CHANNEL_TYPE, QVariant((int)channel_type) );
	reply->setProperty(P_MESSAGE_INDEX, QVariant(message_index) );
}

void MattermostQt::post_send_message(QString message, int server_index, int team_index, int channel_type,
                                     int channel_index)
{
	ServerPtr sc = m_server[server_index];
	//files/{file_id}/thumbnail
	QString urlString = QLatin1String("/api/v")
	        + QString::number(sc->m_api)
	        + QLatin1String("/posts");
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
	requset_set_headers(request,sc);

	QJsonDocument json;
	QJsonObject root;
	root["channel_id"] = channel->m_id;
	root["message"] = message;
	root["files_ids"] = QJsonArray();
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

void MattermostQt::get_user_image(int server_index, int user_index)
{
	if( server_index < 0 || server_index >= m_server.size() )
		return;
	ServerPtr sc = m_server[server_index];
	if( user_index < 0 || user_index >= sc->m_user.size() )
		return;
	UserPtr user = sc->m_user[user_index];

	QString path = sc->m_config_path +
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
	requset_set_headers(request,sc);

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
	requset_set_headers(request,sc);

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

void MattermostQt::get_posts(int server_index, int team_index, int channel_index, int channel_type )
{
	if( server_index < 0 || server_index >= m_server.size() )
		return;
	ServerPtr sc = m_server[server_index];
	ChannelPtr channel;
	if( channel_type == ChannelType::ChannelPublic )
	{
		if( team_index < 0 || team_index >= sc->m_teams.size() )
			return;
		TeamPtr tc =  sc->m_teams[team_index];
		if( channel_index < 0 || channel_index > tc->m_public_channels.size() )
			return;
		channel = tc->m_public_channels[channel_index];
	}
	else if( channel_type == ChannelType::ChannelPrivate )
	{
		if( team_index < 0 || team_index >= sc->m_teams.size() )
			return;
		TeamPtr tc =  sc->m_teams[team_index];
		if( channel_index < 0 || channel_index > tc->m_private_channels.size() )
			return;
		channel = tc->m_private_channels[channel_index];
	}
	else if( channel_type == ChannelType::ChannelDirect)
	{
		if( channel_index < 0 || channel_index > sc->m_direct_channels.size() )
			return;
		channel = sc->m_direct_channels[channel_index];
	}
	else
		return;

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
	requset_set_headers(request,sc);

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
	ChannelPtr channel;
	if( channel_type == ChannelType::ChannelPublic )
	{
		if( team_index < 0 || team_index >= sc->m_teams.size() )
			return;
		TeamPtr tc =  sc->m_teams[team_index];
		if( channel_index < 0 || channel_index > tc->m_public_channels.size() )
			return;
		channel = tc->m_public_channels[channel_index];
	}
	else if( channel_type == ChannelType::ChannelPrivate )
	{
		if( team_index < 0 || team_index >= sc->m_teams.size() )
			return;
		TeamPtr tc =  sc->m_teams[team_index];
		if( channel_index < 0 || channel_index > tc->m_private_channels.size() )
			return;
		channel = tc->m_private_channels[channel_index];
	}
	else if( channel_type == ChannelType::ChannelDirect)
	{
		if( channel_index < 0 || channel_index > sc->m_direct_channels.size() )
			return;
		channel = sc->m_direct_channels[channel_index];
	}
	else
		return;

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
	requset_set_headers(request,sc);

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
	requset_set_headers(request,server);

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
//		server["id"] = (double)sc->m_selfId;
		server["user_id"] = sc->m_user_id;
		server["url"] = sc->m_url;
		server["api"] = (double)sc->m_api;
		server["token"] = sc->m_token;
		server["name"] = sc->m_display_name;
		server["trust_certificate"] = sc->m_trust_cert;
		if(sc->m_trust_cert)
		{
			sc->m_config_path = m_settings_path + QString("%0_%1").arg(i).arg(sc->m_user_id);
			QDir server_dir(sc->m_config_path);
			if(! server_dir.exists() )
				server_dir.mkpath(sc->m_config_path);
			QString new_ca_path = sc->m_config_path + QString("/ca.crt");
			if( QFile::copy(sc->m_ca_cert_path , new_ca_path) )
				sc->m_ca_cert_path = new_ca_path;
			QString new_cert_path = sc->m_config_path + QString("/server.crt");
			if( QFile::copy(sc->m_ca_cert_path , new_cert_path) )
				sc->m_cert_path = new_cert_path;
		}
		server["ca_cert_path"] = sc->m_ca_cert_path;
		server["cert_path"] = sc->m_cert_path;
		server["server_dir"] = sc->m_config_path;
		servers.append(server);
	}
	object["servers"] = servers;
	json.setObject(object);

	QFile jsonFile( m_settings_path + QLatin1String(F_CONFIG_FILE) );
	if( !jsonFile.open(QFile::WriteOnly) )
		return false;
	jsonFile.write(json.toJson());
	jsonFile.close();

	return true;
}

bool MattermostQt::load_settings()
{
	QJsonDocument json;
	QFile jsonFile( m_settings_path + QLatin1String(F_CONFIG_FILE) );
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
		server->m_trust_cert = trust_certificate;
		server->m_display_name = display_name;
		server->m_self_index = m_server.size();
		server->m_ca_cert_path = ca_cert_path;
		server->m_cert_path = cert_path;
		server->m_config_path = object["server_dir"].toString("");
		server->m_user_id = user_id;
		if(server->m_config_path.isEmpty())
			server->m_config_path = m_settings_path + QString("%0_%1").arg(i).arg(user_id);
		m_server.append(server);
		get_login(server);
	}
	return true;
}

void MattermostQt::prepare_direct_channel(int server_index, int team_index, int channel_index)
{
	ChannelPtr ct = m_server[server_index]->m_direct_channels[channel_index];
	ServerPtr sc = m_server[ct->m_server_index];
	/** in name we have two ids, separated with '__' */
	int index = ct->m_name.indexOf("__");
	QString user_id = ct->m_name.left( index );
	if( user_id == sc->m_user_id )
		user_id = ct->m_name.right( ct->m_name.length() - index - 2  );
	// first search in cached users
	for(int i = 0; i < sc->m_user.size(); i++ )
	{
		if( sc->m_user[i]->m_id.compare(user_id) == 0 )
		{
			sc->m_direct_channels[channel_index]->m_display_name = sc->m_user[i]->m_username;
			sc->m_direct_channels[channel_index]->m_dc_user_index = sc->m_user[i]->m_self_index;
			return;
		}
	}
	// send request for user credentials first
	get_user_info(sc->m_self_index, user_id, team_index);
}

void MattermostQt::prepare_user_index(int server_index, MattermostQt::MessagePtr message)
{
	ServerPtr sc;
	if( server_index < 0 || server_index >= m_server.size() )
		return;
	sc = m_server[server_index];

	if( message->m_user_id.isEmpty() )
	{// system message
		message->m_user_index = -1;
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
		get_user_info(server_index, message->m_user_id);
	}
}

void MattermostQt::websocket_connect(ServerPtr server)
{
	// server get us authentificztion token, time to open WebSocket!
	QSharedPointer<QWebSocket> socket(new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this));
	server->m_socket = socket;
	socket->setProperty(P_SERVER_INDEX,server->m_self_index);

	if( server->m_trust_cert )
	{

		QFile ca_cert_file(server->m_ca_cert_path);
		QFile cert_file(server->m_cert_path);
		bool all_ok =
		        ca_cert_file.open(QIODevice::ReadOnly) &
		        cert_file.open(QIODevice::ReadOnly);
		if(all_ok)
		{
			QSslCertificate ca_cert(&ca_cert_file, QSsl::Pem);
			QSslCertificate cert(&cert_file, QSsl::Pem);
			QList<QSslError> errors;
			errors << QSslError(QSslError::CertificateUntrusted, ca_cert);
			errors << QSslError(QSslError::SelfSignedCertificateInChain, ca_cert);
			errors << QSslError(QSslError::SelfSignedCertificate, ca_cert);
			errors << QSslError(QSslError::CertificateUntrusted, cert);
			errors << QSslError(QSslError::SelfSignedCertificateInChain, cert);
			errors << QSslError(QSslError::SelfSignedCertificate, cert);
			socket->ignoreSslErrors(errors);
		}
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

	QNetworkRequest request;
	requset_set_headers(request,server);
	request.setUrl(url);

	socket->open(request);
}

bool MattermostQt::reply_login(QNetworkReply *reply)
{
	if( reply->property(P_SERVER_INDEX).isValid() )
	{
		ServerPtr server;

		int server_id = reply->property(P_SERVER_INDEX).toInt();
		server = m_server[server_id];

		server->m_cert  = reply->sslConfiguration();
		server->m_cookie= reply->header(QNetworkRequest::CookieHeader).toString();

		QJsonDocument json = QJsonDocument::fromJson( reply->readAll() );
		if( json.isObject() )
		{
			QJsonObject object = json.object();
			qDebug() << object;
			server->m_user_id = object["id"].toString();
		}
		websocket_connect(server);
//		emit serverConnected(server->m_self_index);
		return true;
	}
	QList<QByteArray> headerList = reply->rawHeaderList();
	foreach(QByteArray head, headerList) {
		qDebug() << head << ":" << reply->rawHeader(head);
		//search login token
		if( strcmp(head.data(),"Token") == 0 )
		{// yes, auth token founded!
			//add server to server list
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
			m_server.append( server );

			QJsonDocument json = QJsonDocument::fromJson( reply->readAll() );
			if( json.isObject() )
			{
				QJsonObject object = json.object();
				qDebug() << object;
				server->m_user_id = object["id"].toString();
			}
			websocket_connect(server);
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
				tc->save_json( m_server[server_index]->m_config_path );
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
	int team_index = -1; // reply->property(P_TEAM_INDEX).toInt();
	int channel_index = reply->property(P_CHANNEL_INDEX).toInt();
	int channel_type =reply-> property(P_CHANNEL_TYPE).toInt();

	if( server_index < 0 || server_index >= m_server.size() )
		return;
	ServerPtr sc = m_server[server_index];
	ChannelPtr channel;

	if( channel_type == ChannelPublic )
	{
		team_index = reply->property(P_TEAM_INDEX).toInt();
		if( team_index < 0 || team_index >= sc->m_teams.size() )
			return;
		TeamPtr tc =  sc->m_teams[team_index];
		if( channel_index < 0 || channel_index > tc->m_public_channels.size() )
			return;
		channel = tc->m_public_channels[channel_index];
	}
	else if( channel_type == ChannelPrivate)
	{
		team_index = reply->property(P_TEAM_INDEX).toInt();
		if( team_index < 0 || team_index >= sc->m_teams.size() )
			return;
		TeamPtr tc =  sc->m_teams[team_index];
		if( channel_index < 0 || channel_index > tc->m_private_channels.size() )
			return;
		channel = tc->m_private_channels[channel_index];
	}
	else if( channel_type == ChannelDirect )
	{
		if( channel_index < 0 || channel_index > sc->m_direct_channels.size() )
			return;
		channel = sc->m_direct_channels[channel_index];
	}

	QJsonDocument json = QJsonDocument::fromJson(reply->readAll());

//	qDebug() << json;

	QJsonArray order = json.object()["order"].toArray();
	channel->m_message.reserve(order.size());
//	qDebug() << order;
	QJsonObject posts = json.object()["posts"].toObject();
	QJsonObject::iterator it = posts.begin(),
	        end = posts.end();
	bool new_messages = false;
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
		if(message->m_type == MessageType::MessageTypeCount)
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

					for(int k = 0; k < temp->m_file_ids.size(); k++ )
					{
						get_file_info(
						            server_index,
						            team_index,    //temp->m_team_index,
						            channel_type,  //temp->m_channel_type,
						            channel_index, //temp->m_channel_index,
						            temp->m_self_index,
						            temp->m_file_ids[k]);
					}

					// get user_index for post
					prepare_user_index(sc->m_self_index, temp);
					break;
				}
			}
		}
		emit messagesAdded(channel);
	}
}

void MattermostQt::reply_get_posts_before(QNetworkReply *reply)
{
	int server_index = reply->property(P_SERVER_INDEX).toInt();
	int team_index = -1; // reply->property(P_TEAM_INDEX).toInt();
	int channel_index = reply->property(P_CHANNEL_INDEX).toInt();
	int channel_type = reply-> property(P_CHANNEL_TYPE).toInt();
//	int before_message_index = reply-> property(P_MESSAGE_INDEX).toInt();

	if( server_index < 0 || server_index >= m_server.size() )
		return;
	ServerPtr sc = m_server[server_index];
	ChannelPtr channel;

	if( channel_type == ChannelPublic )
	{
		team_index = reply->property(P_TEAM_INDEX).toInt();
		if( team_index < 0 || team_index >= sc->m_teams.size() )
			return;
		TeamPtr tc =  sc->m_teams[team_index];
		if( channel_index < 0 || channel_index > tc->m_public_channels.size() )
			return;
		channel = tc->m_public_channels[channel_index];
	}
	else if( channel_type == ChannelPrivate)
	{
		team_index = reply->property(P_TEAM_INDEX).toInt();
		if( team_index < 0 || team_index >= sc->m_teams.size() )
			return;
		TeamPtr tc =  sc->m_teams[team_index];
		if( channel_index < 0 || channel_index > tc->m_private_channels.size() )
			return;
		channel = tc->m_private_channels[channel_index];
	}
	else if( channel_type == ChannelDirect )
	{
		if( channel_index < 0 || channel_index > sc->m_direct_channels.size() )
			return;
		channel = sc->m_direct_channels[channel_index];
	}

	QVector<MessagePtr> messages;
	QJsonDocument json = QJsonDocument::fromJson(reply->readAll());

	QJsonArray order = json.object()["order"].toArray();
	QJsonObject posts = json.object()["posts"].toObject();
	QJsonObject::iterator it = posts.begin(),
	        end = posts.end();
	messages.reserve(order.size() + channel->m_message.size());
	bool new_messages = false;
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
		if(message->m_type == MessageType::MessageTypeCount)
		{
			if(message->m_user_id.compare(sc->m_user_id) == 0 )
				message->m_type = MessageMine;
			else
				message->m_type = MessageOther;
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
				prepare_user_index(server_index,temp);
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
	for(int i = 0; i < size; i++)
	{
		MessagePtr temp = channel->m_message[i];
		for(int k = 0; k < temp->m_file_ids.size(); k++ )
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
	emit messagesAddedBefore(channel, size);
}

void MattermostQt::reply_get_public_channels(QNetworkReply *reply)
{
	ServerPtr sc;
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
				ct->m_server_index = reply->property(P_SERVER_INDEX).toInt();
				sc.reset();
				sc = m_server[ct->m_server_index];

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
					ct->m_team_index = -1;
					ct->m_self_index = m_server[ct->m_server_index]->m_direct_channels.size();
					m_server[ct->m_server_index]->m_direct_channels.append(ct);
					prepare_direct_channel(ct->m_server_index, ct->m_team_index, ct->m_self_index);
					break;
				default:
					break;
				}
			}
		}
	}
}

void MattermostQt::reply_get_user_info(QNetworkReply *reply)
{
	int server_index = reply->property(P_SERVER_INDEX).toInt();
	int team_index = reply->property(P_TEAM_INDEX).toInt();
	bool direct_channel = reply->property(P_DIRECT_CHANNEL).toBool();

	if( server_index < 0 || server_index >= m_server.size() )
	{
		qWarning() << "Error! Cant find server in servers list!";
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
		get_user_image(server_index,user->m_self_index);
	}

	if(direct_channel && team_index == -1)
	{
		for(int i = 0; i < sc->m_direct_channels.size(); i++)
		{
			ChannelPtr channel = sc->m_direct_channels[i];
			if( channel->m_name.indexOf(user->m_id) >= 0  )
			{
				channel->m_display_name = user->m_username;
				channel->m_dc_user_index = user->m_self_index;
				emit channelAdded(channel);
				break;
			}
		}
	}
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
			if( error_id.compare("api.user.check_user_password.invalid.app_error") == 0 )
			{
				emit connectionError(ConnectionError::WrongPassword, QObject::trUtf8("Login failed because of invalid password") );
			}
		}
	}
	else if ( replyData.size() > 0 )
		qDebug() << replyData.data();
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
	if(team_index >= 0)
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
		QString file_path = sc->m_config_path
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
		QList<MessagePtr> messages;
		messages << mc;
		emit messageUpdated(messages);
	}
	return;
}

void MattermostQt::reply_get_file_info(QNetworkReply *reply)
{
	// we think all indexes right
	int server_index = reply->property(P_SERVER_INDEX).toInt();
	int team_index = reply->property(P_TEAM_INDEX).toInt();
	int channel_type = reply->property(P_CHANNEL_TYPE).toInt();
	int channel_index = reply->property(P_CHANNEL_INDEX).toInt();
	int message_index = reply->property(P_MESSAGE_INDEX).toInt();

	ServerPtr sc = m_server[server_index];
	ChannelPtr channel;
	if(team_index >= 0)
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
	QJsonDocument json = QJsonDocument::fromJson(replyData);
//	qDebug() << json;
	FilePtr file(new FileContainer(json.object()));
	file->m_self_index = mc->m_file.size();
	mc->m_file.append(file);
	file->m_self_sc_index = m_server[server_index]->m_file.size();
	m_server[server_index]->m_file.append(file);
	file->m_server_index = server_index;
	file->m_team_index = team_index;
	file->m_channel_index = channel_index;
	file->m_channel_type = channel_type;
	file->m_message_index = message_index;

	// TODO Here need save file info to json in file's directory
	QString file_thumb_path = sc->m_config_path + QString("/files/%0/thumb.jpeg").arg(file->m_id);
	if( !QFile::exists(file_thumb_path) )
		get_file_thumbnail(server_index,file->m_self_sc_index);
	else {
		file->m_thumb_path = file_thumb_path;
		QList<MessagePtr> messages;
		messages << mc;
		emit messageUpdated(messages);
	}
	return;
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
	qDebug() << replyData;
	{
		QString file_path = sc->m_config_path
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
				qDebug() << file_path;
		}
		else
			user->m_image_path = file_path + QLatin1String("/image.png");
	}
	if(!user->m_image_path.isEmpty())
	{

//		QList<MessagePtr> messages;
//		messages << mc;
//		emit messageUpdated(messages);
		emit userUpdated(user);
	}
//	qDebug() << reply->readAll();

	qDebug() << "get user image";
}

void MattermostQt::reply_post_send_message(QNetworkReply *reply)
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

	if( cmp(ch_type,O) )
		type = ChannelType::ChannelPublic;
	else if( cmp(ch_type,P) )
		type = ChannelType::ChannelPrivate;
	else if( cmp(ch_type,D) )
		type = ChannelType::ChannelDirect;

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
			message->m_server_index  = sc->m_self_index;
			message->m_team_index    = -1;
			message->m_channel_index = channel_index;
			message->m_self_index    = channel->m_message.size();
			channel->m_message.append(message);
			if(message->m_type == MessageType::MessageTypeCount)
			{
				if(message->m_user_id.compare(sc->m_user_id) == 0 )
					message->m_type = MessageMine;
				else
					message->m_type = MessageOther;
			}
			prepare_user_index(sc->m_self_index, message);
			for(int k = 0; k < message->m_file_ids.size(); k++ )
			{
				get_file_info(sc->m_self_index,-1,(int)ChannelType::ChannelDirect,channel_index,
				                   message->m_self_index, message->m_file_ids[k]);
			}
			QList<MessagePtr> new_messages;
			new_messages << message;
			emit messageAdded(new_messages);
		}
	}
	else
	{
		QString team_id = data["team_id"].toString();
		QString channel_id = message->m_channel_id;
		int team_index = -1;
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
			message->m_server_index  = sc->m_self_index;
			message->m_team_index    = -1;
			message->m_channel_index = channel_index;
			message->m_self_index    = channel->m_message.size();
			channel->m_message.append(message);
			if(message->m_type == MessageType::MessageTypeCount)
			{
				if(message->m_user_id.compare(sc->m_user_id) == 0 )
					message->m_type = MessageMine;
				else
					message->m_type = MessageOther;
			}
			prepare_user_index(sc->m_self_index, message);
			for(int k = 0; k < message->m_file_ids.size(); k++ )
			{
				get_file_info(sc->m_self_index,team_index,type,channel_index,
				                   message->m_self_index, message->m_file_ids[k]);
			}
			QList<MessagePtr> new_messages;
			new_messages << message;
			emit messageAdded(new_messages);
		}
	}
}

void MattermostQt::event_post_edited(MattermostQt::ServerPtr sc, QJsonObject data)
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

	if( cmp(ch_type,O) )
		type = ChannelType::ChannelPublic;
	else if( cmp(ch_type,P) )
		type = ChannelType::ChannelPrivate;
	else if( cmp(ch_type,D) )
		type = ChannelType::ChannelDirect;

	ChannelPtr channel;
	int channel_index = -1;

	if( type == ChannelType::ChannelDirect )
	{
		QString channel_name = data["channel_name"].toString();
		for(int i = 0; i < sc->m_direct_channels.size(); i++ )
		{
			if( scmp(sc->m_direct_channels[i]->m_name,channel_name) )
			{
				channel = sc->m_direct_channels[i];
				channel_index = i;
				break;
			}
		}
	}
	else
	{
		QString team_id = data["team_id"].toString();
		QString channel_id = message->m_channel_id;
		int team_index = -1;

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
	}
	if( channel && channel_index >= 0)
	{
		return;
	}
}

void MattermostQt::event_post_deleted(MattermostQt::ServerPtr sc, QJsonObject data)
{

}

void MattermostQt::replyFinished(QNetworkReply *reply)
{
	if (reply->error() == QNetworkReply::NoError) {
		//success
		QVariant replyType;
		replyType = reply->property(P_REPLY_TYPE);

		if(reply->header(QNetworkRequest::LastModifiedHeader).isValid())
			qDebug() << "LastModified" << reply->header(QNetworkRequest::LastModifiedHeader);

		if(replyType.isValid())
		{
			switch (replyType.toInt()) {
			case ReplyType::Login:
				if( reply_login(reply) )
				{//connect timers
//					connect( &m_update_server, SIGNAL(timeout()), SLOT(slot_get_teams_unread()) );
//					m_update_server.setTimerType(Qt::/*TimerType*/);
//					m_update_server.start();
//					slot_get_teams_unread();
				}
				break;
			case ReplyType::Teams:
				reply_get_teams(reply);
				break;
			case ReplyType::Channels:
				reply_get_public_channels(reply);
				break;
			case ReplyType::rt_get_user_info:
				reply_get_user_info(reply);
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
			case ReplyType::rt_get_file_info:
				reply_get_file_info(reply);
				break;
			case ReplyType::rt_post_send_message:
				reply_post_send_message(reply);
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
	}
	delete reply;
}

void MattermostQt::replySSLErrors(QNetworkReply *reply, QList<QSslError> errors)
{
	bool trustCertificate = false;
	QVariant ts = reply->property(P_SERVER_INDEX);
	if( ts.isValid() )
		trustCertificate = m_server[ts.toInt()]->m_trust_cert;
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
//	int err = 0;
	foreach(QSslError error, errors)
	{
//		err = error.error();
		qWarning() << (int)error.error() << error.errorString();
	}

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
	qDebug() << message;

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

	_compare(hello) // that mean we are logged in
	{
		qDebug() << event;
		save_settings();
		emit serverConnected(sc->m_self_index);
	}
	else _compare(posted)
	    event_posted(sc,data);
	else _compare(post_edited)
	    event_post_edited(sc,data);
	else
	    qWarning() << event;
//typing
//post_edited
//post_deleted
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
//status_change
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
		if( m_server[i]->m_state == ServerUnconnected )
		{
			QString urlString = QLatin1String("/api/v")
			        + QString::number(m_server[i]->m_api)
			        + QLatin1String("/websocket");

			QString serUrl = m_server[i]->m_url;
			serUrl.replace("https://","wss://")
			        .replace("http://","ws://");
			QUrl url(serUrl);
			url.setPath(urlString);

			QNetworkRequest request;
			requset_set_headers(request,m_server[i]);
			request.setUrl(url);

			m_server[i]->m_socket->open(request);
		}
	}
}

MattermostQt::TeamContainer::TeamContainer(QJsonObject &object)
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

MattermostQt::ChannelContainer::ChannelContainer(QJsonObject &object)
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

MattermostQt::UserContainer::UserContainer(QJsonObject object)
{
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
	//"last_password_update": 0,
	m_last_password_update = (qlonglong)object["last_password_update"].toDouble();
	//"last_picture_update": 0,
	m_last_picture_update = (qlonglong)object["last_picture_update"].toDouble();
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
		m_type = MessageType::MessageSystem;
	else
		m_type = MessageType::MessageTypeCount;
	QJsonArray filenames = object["filenames"].toArray();
	QJsonArray file_ids = object["file_ids"].toArray();
	for(int i = 0; i < filenames.size(); i++ )
		m_filenames.append( filenames.at(i).toString() );
	for(int i = 0; i < file_ids.size(); i++ )
		m_file_ids.append( file_ids.at(i).toString() );
}

MattermostQt::FileContainer::FileContainer(QJsonObject object)
{
	qDebug() << object;
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
