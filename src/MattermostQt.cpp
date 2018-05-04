#include "MattermostQt.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QStandardPaths>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>

//#include "libs/qtwebsockets/include/QtWebSockets/qwebsocket.h"
//#include <QtWebSockets>

#define P_REPLY_TYPE        "reply_type"
#define P_API               "api"
#define P_SERVER_URL        "server_url"
#define P_SERVER_INDEX         "server_index"
#define P_TEAM_INDEX        "team_index"
#define P_TEAM_ID           "team_id"
#define P_CHANNEL_INDEX     "channel_id"
#define P_CHANNEL_TYPE      "channel_type"
#define P_TRUST_CERTIFICATE "trust_certificate"

#define F_CONFIG_FILE       "config.json"


#define requset_set_headers(requset, server) \
	request.setHeader(QNetworkRequest::ServerHeader, "application/json"); \
	request.setHeader(QNetworkRequest::UserAgentHeader, QString("MattermosQt v%0").arg(MATTERMOSTQT_VERSION) ); \
	request.setHeader(QNetworkRequest::CookieHeader, server->m_cookie); \
	request.setRawHeader("Authorization", QString("Bearer %0").arg(server->m_token).toUtf8())

MattermostQt::MattermostQt()
{
	m_networkManager.reset(new QNetworkAccessManager());

	m_update_server_timeout = 5000; // in millisecs
	m_update_server.setInterval(m_update_server_timeout);

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

}

void MattermostQt::post_login(QString server, QString login, QString password, bool trustCertificate, int api)
{
	if(api <= 3)
		api = 4;

//#if defined(SERVER_URL) && defined(_DEBUG)
//	server = QString(SERVER_URL);
//	login = "testuser";
//	password = "testuser";
//	trustCertificate = true;
//#endif

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
	reply->setProperty(P_SERVER_URL, server);
	reply->setProperty(P_TRUST_CERTIFICATE, trustCertificate);

//	// Load previosly saved certificate
//	QFile certFile(SSLCERTIFICATE);
//	certFile.open(QIODevice::ReadOnly);
//	QSslCertificate cert(&certFile, QSsl::Pem);
//	QSslSocket * sslSocket = new QSslSocket(this);
//	sslSocket->addCaCertificate(cert);
//	QSslConfiguration configuration = sslSocket->sslConfiguration();
//	configuration.setProtocol(QSsl::TlsV1_2);

//	sslSocket->setSslConfiguration(configuration);
	//	reply->setSslConfiguration(configuration);
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
}

void MattermostQt::get_teams(int serverId)
{
	QMap<int,ServerPtr>::iterator it = m_server.find(serverId);
	if( it == m_server.end() )
		return;
	ServerPtr sc = it.value();

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
	reply->setProperty(P_SERVER_INDEX, QVariant(serverId) );
}

void MattermostQt::get_public_channels(int server_index, QString team_id)
{
	if( team_id.isNull() || team_id.isEmpty() || server_index < 0 )
	{
		qWarning() << "Wrong team id";
		return;
	}

	QMap<int,ServerPtr>::iterator it = m_server.find(server_index);
	if( it == m_server.end() )
		return;
	ServerPtr sc = it.value();
	// first check if team allready got channels
	int team_index = sc->get_team_index(team_id);
	if(team_index == -1)
	{
		qWarning() << "Team with id " << team_id << " not found";
		return;
	}

	TeamPtr tc = sc->m_teams[team_index];
	if( tc->m_private_channels.size() + tc->m_public_channels.size() + tc->m_direct_channels.size() > 0 )
	{
		QList<ChannelPtr> channels;

		for(int i = 0; i < tc->m_public_channels.size(); i++ )
			channels.append(tc->m_public_channels[i]);

		for(int i = 0; i < tc->m_private_channels.size(); i++ )
			channels.append(tc->m_private_channels[i]);

		for(int i = 0; i < tc->m_direct_channels.size(); i++ )
			channels.append(tc->m_direct_channels[i]);

		emit channelsList( channels );
		// after that send request for team info

		get_team(server_index,team_index);
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

void MattermostQt::get_user_image(int serverId, QString userId)
{
	// TODO
}

void MattermostQt::get_user_info(int serverId, QString userId)
{
	QMap<int,ServerPtr>::iterator it = m_server.find(serverId);
	if( it == m_server.end() )
		return;
	ServerPtr sc = it.value();

	QString urlString = QLatin1String("/api/v")
	        + QString::number(sc->m_api)
	        + QLatin1String("/users/me/teams/")
	        + userId;

	QUrl url(sc->m_url);
	url.setPath(urlString);
	QNetworkRequest request;

	request.setUrl(url);
	requset_set_headers(request,sc);

	if(sc->m_trust_cert)
		request.setSslConfiguration(sc->m_cert);

	QNetworkReply *reply = m_networkManager->get(request);
	reply->setProperty(P_REPLY_TYPE, QVariant(ReplyType::User) );
	reply->setProperty(P_SERVER_INDEX, QVariant(serverId) );
}

void MattermostQt::get_teams_unread(int server_index)
{
	QMap<int,ServerPtr>::iterator it = m_server.find(server_index);
	if( it == m_server.end() )
		return;
	get_teams_unread(it.value());
}

void MattermostQt::get_teams_unread(MattermostQt::ServerPtr server)
{
	// request uri users/{user_id}/teams/unread
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
		server["trust_certificate"] = sc->m_trust_cert;

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
		bool trust_certificate = object["trust_certificate"].toBool();

		if( user_id.isEmpty() || url.isEmpty() || token.isEmpty() )
			return false;

		// create server container
		ServerPtr server( new ServerContainer(url,token,api) );
		server->m_trust_cert = trust_certificate;

		server->m_self_index = m_server.size();
		m_server[server->m_self_index] = server;
		get_login(server);
	}
}

void MattermostQt::prepare_direct_channel(int server_index, int team_index, int channel_index)
{
	ChannelPtr ct = m_server[server_index]->m_teams[team_index]->m_direct_channels[channel_index];
	ServerPtr sc = m_server[ct->m_server_index];
	/** in name we have two ids, separated with '__' */
	int index = ct->m_name.indexOf("__");
	QString id1 = ct->m_name.left( index );
	if( id1 == sc->m_user_id )
		id1 = ct->m_name.right( ct->m_name.length() - index - 2  );
	// first search in cached users

	// send request for user credentials first
	QString urlString = QLatin1String("/api/v")
	        + QString::number(sc->m_api)
	        + QLatin1String("/users/")
	        + id1;
	QUrl url(sc->m_url);
	url.setPath(urlString);

	QNetworkRequest request;

	request.setUrl(url);
	requset_set_headers(request,sc);
	if(sc->m_trust_cert)
		request.setSslConfiguration(sc->m_cert);

	QNetworkReply *reply = m_networkManager->get(request);
	reply->setProperty(P_REPLY_TYPE, QVariant(ReplyType::User) );
	reply->setProperty(P_SERVER_INDEX, QVariant(ct->m_server_index) );
	reply->setProperty(P_TEAM_INDEX, QVariant(ct->m_team_index) );
}

void MattermostQt::websocket_connect(ServerPtr server)
{
	// server get us authentificztion token, time to open WebSocket!
	QSharedPointer<QWebSocket> socket(new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this));
	socket->setProperty(P_SERVER_INDEX,server->m_self_index);
	// ignore allready trusted certificate
	QList<QSslCertificate> cert = server->m_cert.caCertificates();
	QList<QSslError> expectedSslErrors;
	expectedSslErrors.append( QSslError(QSslError::SelfSignedCertificate, cert.at(0)) );
	// TODO - remove HostNameMistchmach
	expectedSslErrors.append( QSslError(QSslError::HostNameMismatch, cert.at(0)) );
	socket->ignoreSslErrors(expectedSslErrors);

	connect(socket.data(), SIGNAL(connected()), SLOT(onWebSocketConnected()));
	typedef void (QWebSocket:: *sslErrorsSignal)(const QList<QSslError> &);
	connect(socket.data(), static_cast<sslErrorsSignal>(&QWebSocket::sslErrors),
	        this, &MattermostQt::onWebSocketSslError);
	connect(socket.data(), SIGNAL(error(QAbstractSocket::SocketError)),
	        SLOT(onWebSocketError(QAbstractSocket::SocketError)));

	QString urlString = QLatin1String("/api/v")
	        + QString::number(server->m_api)
	        + QLatin1String("/websocket");
/// requset json sample
//			{
//			  "seq": 1,
//			  "action": "authentication_challenge",
//			  "data": {
//			    "token": "mattermosttokengoeshere"
//			  }
//			}
	QString serUrl = server->m_url.replace("https://","wss://")
	        .replace("http://","ws://");
	QUrl url("ws://echo.websocket.org");
	//url.setPath(urlString.append("/"));

	QNetworkRequest request;
//			QJsonDocument json;
//			QJsonObject root;
//			QJsonObject data;

//			root["seq"] = 1;
//			root["action"] = "authentication_challenge";
//			data["token"] = newServer.m_token;
//			root["data"] = data;
//			json.setObject(data);

	request.setUrl(url);
//			request.setHeader(QNetworkRequest::ServerHeader, "application/json");
//	request.setHeader(QNetworkRequest::UserAgentHeader, QString("MattermosQt v%0").arg(MATTERMOSTQT_VERSION) );
//	request.setHeader(QNetworkRequest::CookieHeader, server.m_cookie );
//			request.setRawHeader("Authorization", QString("Bearer %0").arg(newServer.m_token).toUtf8());
//			request.setHeader(QNetworkRequest::ContentLengthHeader, QByteArray::number( json.toJson().size() ));

	socket->open(url);
	//server.m_socket = socket;
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

		emit serverConnected(server->m_self_index);
		return true;
	}
	QList<QByteArray> headerList = reply->rawHeaderList();
	foreach(QByteArray head, headerList) {
		qDebug() << head << ":" << reply->rawHeader(head);
		//search login token
		if( strcmp(head.data(),"Token") == 0 )
		{// yes, auth token founded!
			//add server to server list

			int server_id = -1;
//			bool is_new_server = false;
			ServerPtr server;
//			if( reply->property(P_SERVER_ID).isValid() )
//			{
//				server_id = reply->property(P_SERVER_ID).toInt();
//				server = m_server[server_id];
//			}
//			else
//			{
			    server_id = m_server.size();
				server.reset(new ServerContainer());
				server->m_api   = reply->property(P_API).toInt();
				server->m_url   = reply->property(P_SERVER_URL).toString();
				server->m_token = reply->rawHeader(head).data();
				server->m_trust_cert = reply->property(P_TRUST_CERTIFICATE).toBool();
				server->m_self_index    = server_id;
//				is_new_server = true;
//			}

			server->m_cert  = reply->sslConfiguration();
			server->m_cookie= reply->header(QNetworkRequest::CookieHeader).toString();
//			if(is_new_server)
			    m_server[server_id] = server;

			QJsonDocument json = QJsonDocument::fromJson( reply->readAll() );
			if( json.isObject() )
			{
				QJsonObject object = json.object();
				qDebug() << object;
				server->m_user_id = object["id"].toString();
			}
//			websocket_connect(newServer);
//			if(is_new_server)
			    save_settings();

			emit serverConnected(server->m_self_index);
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
				m_server[serverId]->m_teams << tc;
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

void MattermostQt::reply_get_public_channels(QNetworkReply *reply)
{
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
				ct->m_team_index = reply->property(P_TEAM_INDEX).toInt();
				ct->m_server_index = reply->property(P_SERVER_INDEX).toInt();
				TeamPtr tc = m_server[ct->m_server_index]->m_teams[ct->m_team_index];
				// open channels
				if( ct->m_type.compare("O") == 0 )
				{
					ct->m_self_index = tc->m_public_channels.size();
					tc->m_public_channels << ct;
					emit channelAdded(ct);
				}
				// private channels
				else if( ct->m_type.compare("P") == 0 )
				{
					ct->m_self_index = tc->m_private_channels.size();
					tc->m_private_channels << ct;
					emit channelAdded(ct);
				}
				// direct channel
				else if ( ct->m_type.compare("D") == 0 )
				{
					ct->m_self_index = tc->m_direct_channels.size();
					tc->m_direct_channels << ct;
					prepare_direct_channel(ct->m_server_index, ct->m_team_index, ct->m_self_index);
				}
			}
		}
	}
}

void MattermostQt::reply_get_user(QNetworkReply *reply)
{
	QJsonDocument json = QJsonDocument::fromJson(reply->readAll());

	qDebug() << json;

	if( !json.isObject() )
	{
		qWarning() << "Cant parse Json";
		return;
	}

	UserPtr user( new UserContainer(json.object()) );
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
					connect( &m_update_server, SIGNAL(timeout()), SLOT(slot_get_teams_unread()) );
//					m_update_server.setTimerType(Qt::/*TimerType*/);
					m_update_server.start();
				}
				break;
			case ReplyType::Teams:
				reply_get_teams(reply);
				break;
			case ReplyType::Channels:
				reply_get_public_channels(reply);
				break;
			case ReplyType::User:
				reply_get_user(reply);
				break;
			case ReplyType::rt_get_team:
				reply_get_team(reply);
				break;
			case ReplyType::rt_get_teams_unread:
				reply_get_teams_unread(reply);
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
				// TODO - remove HostNameMistchmach
			case QSslError::HostNameMismatch:
				ignoreErrors << error;
				break;
			default:
				ignoreErrors << error;
				break;
			}
		}
//		reply->ignoreSslErrors(ignoreErrors);
		reply->ignoreSslErrors(errors);
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
	int id = sId.toInt();
	QMap<int,ServerPtr>::iterator it = m_server.find(id);
	if( it == m_server.end() ) // that really strange! whats wrong? how it could be?
		return;

//	ServerPtr sc = it.value();

	emit serverConnected(id);
}

void MattermostQt::onWebSocketSslError(QList<QSslError> errors)
{
	foreach(QSslError error, errors)
	{
		qWarning() << error;
	}
}

void MattermostQt::onWebSocketError(QAbstractSocket::SocketError error)
{
	qWarning() << error;
	QWebSocket * socket = qobject_cast<QWebSocket*>(sender());
	if(!socket) // strange situation, if it happens
		return;

	qWarning() << socket->errorString();
//	QNetworkRequest req = socket->request();
	//	req
}

void MattermostQt::slot_get_teams_unread()
{
	foreach(ServerPtr server, m_server)
	{
		get_teams_unread(server);
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
	m_type = object["type"].toString();
	m_display_name = object["display_name"].toString();
	m_name = object["name"].toString();
	m_header = object["header"].toString();
	m_purpose = object["purpose"].toString();
	m_last_post_at = (qlonglong)object["last_post_at"].toDouble();
	m_total_msg_count = (qlonglong)object["total_msg_count"].toDouble();
	m_extra_update_at = (qlonglong)object["extra_update_at"].toDouble();
	m_creator_id = object["creator_id"].toString();
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
