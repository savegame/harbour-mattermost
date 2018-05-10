#include "MattermostQt.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QStandardPaths>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>

//#include "libs/qtwebsockets/include/QtWebSockets/qwebsocket.h"
//#include <QtWebSockets>

#define P_REPLY_TYPE         "reply_type"
#define P_API                "api"
#define P_SERVER_URL         "server_url"
#define P_SERVER_NAME        "server_name"
#define P_SERVER_INDEX       "server_index"
#define P_TEAM_INDEX         "team_index"
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
#define comare(string) if( cmp(event,string) )

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
	if( server_index < 0 || server_index >= m_server.size() )
		return;
	ServerPtr sc = m_server[server_index];

	QString urlString = QLatin1String("/api/v")
	        + QString::number(sc->m_api)
	        + QLatin1String("/users/me/teams");

	QUrl url(sc->m_url);
	url.setPath(urlString);
	QNetworkRequest request;
	QJsonDocument json;
	QJsonObject data;

//	data["page"] = 0;
//	data["per_page"] = 10;

	//json.setObject(data);

	request.setUrl(url);
	requset_set_headers(request,sc);
//	request.setHeader(QNetworkRequest::ContentLengthHeader, QByteArray::number( json.toJson().size() ));

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

void MattermostQt::get_user_image(int server_index, QString user_id)
{
	Q_UNUSED(server_index)
	Q_UNUSED(user_id)
	// TODO
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
	reply->setProperty(P_REPLY_TYPE, QVariant(ReplyType::get_user) );
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
	if( team_index < 0 || team_index >= sc->m_teams.size() )
		return;
	TeamPtr tc =  sc->m_teams[team_index];
	ChannelPtr channel;
	if( channel_type == ChannelType::ChannelPublic )
	{
		if( channel_index < 0 || channel_index > tc->m_public_channels.size() )
			return;
		channel = tc->m_public_channels[channel_index];
	}
	else if( channel_type == ChannelType::ChannelPrivate )
	{
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
	QString per_page("\"20\"");
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
	reply->setProperty(P_CHANNEL_TYPE, QVariant(channel->m_type) );
	reply->setProperty(P_CHANNEL_INDEX, QVariant(channel_index) );
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
//			QFile ca_cert_file(sc->m_ca_cert_path);
			QString server_dir_path = m_settings_path + QString("%0_%1").arg(i).arg(sc->m_user_id);
//			QFile::exists()
//			if( ca_cert_file.open(QIODevice::ReadOnly) )
//			{
			    QDir server_dir(server_dir_path);

				if(! server_dir.exists() )
				{
					server_dir.mkpath(server_dir_path);
				}
				QString new_ca_path = server_dir_path + QString("/ca.crt");
				if( QFile::copy(sc->m_ca_cert_path , new_ca_path) )
				    sc->m_ca_cert_path = new_ca_path;
//				QFile save_cert( new_ca_path );
//				save_cert.open(QIODevice::WriteOnly);
//				save_cert.write( ca_cert_file.readAll() );
//				save_cert.close();
//			}
//			QFile cert_file(sc->m_cert_path);
//			if( cert_file.open(QIODevice::ReadOnly) )
//			{
				QString new_cert_path = server_dir_path + QString("/server.crt");
				if( QFile::copy(sc->m_ca_cert_path , new_cert_path) )
				    sc->m_cert_path = new_cert_path;
//				QFile save_cert( new_cert_path );
//				save_cert.open(QIODevice::WriteOnly);
//				save_cert.write( cert_file.readAll() );
//				save_cert.close();
//			}
		}
		server["ca_cert_path"] = sc->m_ca_cert_path;
		server["cert_path"] = sc->m_cert_path;
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

	// send request for user credentials first
	get_user_info(sc->m_self_index, user_id, team_index);
	// and send request for user picture
	get_user_image(sc->m_self_index, user_id);
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
	int serverId = reply->property(P_SERVER_INDEX).toInt();
	json = QJsonDocument::fromJson(rawData);

	if( !json.isEmpty() && json.isArray() ) {
		QJsonArray array = json.array();
		for( int i = 0 ; i < array.size(); i ++ )
		{
			if( array.at(i).isObject() )
			{
				QJsonObject object = array.at(i).toObject();
				TeamPtr tc(new TeamContainer(object) );
				tc->m_server_index = serverId;
				tc->m_self_index = m_server[serverId]->m_teams.size();
				m_server[serverId]->m_teams.append(tc);
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

	if( server_index < 0 || server_index >= m_server.size() )
		return;
	ServerPtr sc = m_server[server_index];
	if( team_index < 0 || team_index >= sc->m_teams.size() )
		return;
	TeamPtr tc =  sc->m_teams[team_index];
	ChannelPtr channel;
//	ChannelType channelType;
	if( channel_type == ChannelPublic )
	{
		if( channel_index < 0 || channel_index > tc->m_public_channels.size() )
			return;
		channel = tc->m_public_channels[channel_index];
//		channelType = ChannelType::ChannelPublic;
	}
	else if( channel_type == ChannelPrivate)
	{
		if( channel_index < 0 || channel_index > tc->m_private_channels.size() )
			return;
		channel = tc->m_private_channels[channel_index];
//		channelType = ChannelType::ChannelPrivate;
	}
	else if( channel_type == ChannelDirect )
	{
		if( channel_index < 0 || channel_index > sc->m_direct_channels.size() )
			return;
		channel = sc->m_direct_channels[channel_index];
//		channelType = ChannelType::ChannelDirect;
	}

	QJsonDocument json = QJsonDocument::fromJson(reply->readAll());

	qDebug() << json;

	QJsonArray order = json.object()["order"].toArray();
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
		QVector<MessagePtr> sort_messages;
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
					break;
				}
			}
		}
		emit messagesAdded(channel);
	}
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

void MattermostQt::reply_get_user(QNetworkReply *reply)
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
	if(!message)
		return;

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
			QList<MessagePtr> new_messages;
			new_messages << message;
			emit messageAdded(new_messages);
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
			QList<MessagePtr> new_messages;
			new_messages << message;
			emit messageAdded(new_messages);
		}
	}
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
					slot_get_teams_unread();
				}
				break;
			case ReplyType::Teams:
				reply_get_teams(reply);
				break;
			case ReplyType::Channels:
				reply_get_public_channels(reply);
				break;
			case ReplyType::get_user:
				reply_get_user(reply);
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

	comare(hello) // that mean we are logged in
	{
		qDebug() << event;
		save_settings();
		emit serverConnected(sc->m_self_index);
	}
	else comare(posted)
	    event_posted(sc,data);
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
	qDebug() << object;
	m_id = object["id"].toString();
	m_channel_id = object["channel_id"].toString();
	m_message = object["message"].toString();
	m_type_string = object["type"].toString();
	m_user_id = object["user_id"].toString();
	if( m_type_string.indexOf("system_") >= 0 )
		m_type = MessageType::MessageSystem;
	else
		m_type = MessageType::MessageTypeCount;
//	QString sender_name = object["sender_name"].toString();
	QJsonArray filenames = object["filenames"].toArray();
	QJsonArray file_ids = object["file_ids"].toArray();
	for(int i = 0; i < filenames.size(); i++ )
		m_filenames.append( filenames.at(i).toString() );
	for(int i = 0; i < file_ids.size(); i++ )
		m_file_ids.append( file_ids.at(i).toString() );

	qDebug() << QString("%0 : %1").arg(m_user_id).arg(m_message);
//	QVector<FilePtr> m_file;
//	QString          m_id;
//	QString          m_channel_id;
//	QString          m_type
}
