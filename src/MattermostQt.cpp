#include "MattermostQt.h"

#include <QNetworkRequest>
#include <QNetworkReply>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>


#define P_REPLY_TYPE        "reply_type"
#define P_API               "api"
#define P_SERVER_URL        "server_url"
#define P_SERVER_ID         "server_id"
#define P_TEAM_INDEX        "team_id"
#define P_TRUST_CERTIFICATE "trust_certificate"

MattermostQt::MattermostQt()
{
	m_networkManager.reset(new QNetworkAccessManager());

	connect(m_networkManager.data(), SIGNAL(finished(QNetworkReply*)),
	        this, SLOT(replyFinished(QNetworkReply*)));

	connect(m_networkManager.data(),SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)),
	        this, SLOT(replySSLErrors(QNetworkReply*,QList<QSslError>)));
}

MattermostQt::~MattermostQt()
{

}

void MattermostQt::post_login(QString server, QString login, QString password, bool trustCertificate, int api)
{
	if(api <= 3)
		api = 4;

#if defined(SERVER_URL) && defined(_DEBUG)
	server = QString(SERVER_URL);
	login = "testuser";
	password = "testuser";
	trustCertificate = true;
#endif

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

//	// Connection via HTTPS
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

void MattermostQt::get_teams(int serverId)
{
	QMap<int,ServerContainer>::iterator it = m_server.find(serverId);
	if( it == m_server.end() )
		return;
	ServerContainer sc = it.value();

	QString urlString = QLatin1String("/api/v")
	        + QString::number(sc.m_api)
	        + QLatin1String("/users/me/teams");

	QUrl url(sc.m_url);
	url.setPath(urlString);
	QNetworkRequest request;
	QJsonDocument json;
	QJsonObject data;

//	data["page"] = 0;
//	data["per_page"] = 10;

	//json.setObject(data);

	request.setUrl(url);
	request.setHeader(QNetworkRequest::ServerHeader, "application/json");
	request.setHeader(QNetworkRequest::UserAgentHeader, QString("MattermosQt v%0").arg(MATTERMOSTQT_VERSION) );
//	request.setRawHeader("X-Custom-User-Agent", QString("MattermosQt v%0").arg(MATTERMOSTQT_VERSION).toUtf8());

	request.setRawHeader("Authorization", QString("Bearer %0").arg(sc.m_token).toUtf8());
//	request.setHeader(QNetworkRequest::ContentLengthHeader, QByteArray::number( json.toJson().size() ));

	if(sc.m_trustCertificate)
		request.setSslConfiguration(sc.m_cert);

	QNetworkReply *reply = m_networkManager->get(request);
	reply->setProperty(P_REPLY_TYPE, QVariant(ReplyType::Teams) );
	reply->setProperty(P_SERVER_ID, QVariant(serverId) );
}

void MattermostQt::get_public_channels(int serverId, QString teamId)
{
	if( teamId.isNull() || teamId.isEmpty() || serverId < 0 )
	{
		qWarning() << "Wrong team id";
		return;
	}

	QMap<int,ServerContainer>::iterator it = m_server.find(serverId);
	if( it == m_server.end() )
		return;
	ServerContainer sc = it.value();

	QString urlString = QLatin1String("/api/v")
	        + QString::number(sc.m_api)
	        + QLatin1String("/users/me/teams/")
	        + teamId
	        + QLatin1String("/channels");

	QUrl url(sc.m_url);
	url.setPath(urlString);
	QNetworkRequest request;

	request.setUrl(url);
	request.setHeader(QNetworkRequest::ServerHeader, "application/json");
	request.setHeader(QNetworkRequest::UserAgentHeader, QString("MattermosQt v%0").arg(MATTERMOSTQT_VERSION) );
//	request.setRawHeader("X-Custom-User-Agent", QString("MattermosQt v%0").arg(MATTERMOSTQT_VERSION).toUtf8());

	request.setRawHeader("Authorization", QString("Bearer %0").arg(sc.m_token).toUtf8());

	if(sc.m_trustCertificate)
		request.setSslConfiguration(sc.m_cert);

	QNetworkReply *reply = m_networkManager->get(request);
	reply->setProperty(P_REPLY_TYPE, QVariant(ReplyType::Team) );
	reply->setProperty(P_SERVER_ID, QVariant(serverId) );
	reply->setProperty(P_TEAM_INDEX, QVariant(teamId) );
}

void MattermostQt::saveSettings()
{

}

bool MattermostQt::reply_login(QNetworkReply *reply)
{
	QList<QByteArray> headerList = reply->rawHeaderList();
	foreach(QByteArray head, headerList) {
		qDebug() << head << ":" << reply->rawHeader(head);
		//search login token
		if( strcmp(head.data(),"Token") == 0 )
		{// yes, auth token founded!
			//add server to server list
			int servers_count = m_server.size();

			ServerContainer newServer;
			newServer.m_api   = reply->property(P_API).toInt();
			newServer.m_url   = reply->property(P_SERVER_URL).toString();
			newServer.m_token = reply->rawHeader(head).data();
			newServer.m_cert  = reply->sslConfiguration();
			newServer.m_trustCertificate = reply->property(P_TRUST_CERTIFICATE).toBool();

//			// server get us authentificztion token, time to open WebSocket!
//			QSharedPointer<QWebSocket> socket(new QWebSocket());
//			socket->setProperty(P_SERVER_ID,servers_count);
//			// ignore allready trusted certificate
//			QList<QSslCertificate> cert = newServer.m_cert.caCertificates();
//			QList<QSslError> expectedSslErrors;
//			expectedSslErrors.append( QSslError(QSslError::SelfSignedCertificate, cert.at(0)) );
//			// TODO - remove HostNameMistchmach
//			expectedSslErrors.append( QSslError(QSslError::HostNameMismatch, cert.at(0)) );
//;
//			socket->ignoreSslErrors(expectedSslErrors);

//			connect(socket.data(), SIGNAL(connected()), SLOT(onWebSocketConnected()));
//			typedef void (QWebSocket:: *sslErrorsSignal)(const QList<QSslError> &);
//			connect(socket.data(), static_cast<sslErrorsSignal>(&QWebSocket::sslErrors),
//			        this, &MattermostQt::onWebSocketSslError);
//			connect(socket.data(), SIGNAL(error(QAbstractSocket::SocketError)),
//			        SLOT(onWebSocketError(QAbstractSocket::SocketError)));

//			QString urlString = QLatin1String("/api/v")
//			        + QString::number(newServer.m_api)
//			        + QLatin1String("/websocket");

////			{
////			  "seq": 1,
////			  "action": "authentication_challenge",
////			  "data": {
////			    "token": "mattermosttokengoeshere"
////			  }
////			}

//			QUrl url(newServer.m_url);
//			url.setPath(urlString);

////			QNetworkRequest request;
//			QJsonDocument json;
//			QJsonObject root;
//			QJsonObject data;

//			root["seq"] = 1;
//			root["action"] = "authentication_challenge";
//			data["token"] = newServer.m_token;
//			root["data"] = data;
//			json.setObject(data);

//			url.setScheme("application/json");
////			url.setQuery(json.toJson().data());
////			socket-
//			socket->ping( json.toJson().data() );
//			socket->open(url);
//			socket->ping( json.toJson().data() );
//			newServer.m_socket = socket;

			m_server[servers_count++] = newServer;
			emit serverConnected(servers_count - 1);
		}
	}
}

void MattermostQt::reply_get_teams(QNetworkReply *reply)
{
	QJsonDocument json;
	QByteArray rawData = reply->readAll();
	int serverId = reply->property(P_SERVER_ID).toInt();
	json = QJsonDocument::fromJson(rawData);

	if( !json.isEmpty() && json.isArray() ) {
		QJsonArray array = json.array();
		for( int i = 0 ; i < array.size(); i ++ )
		{
			if( array.at(i).isObject() )
			{
				QJsonObject object = array.at(i).toObject();
				TeamContainer team(object);
				team.m_serverId = serverId;
				m_server[serverId].m_teams << team;
				emit teamAdded(team);
			}
			else
				qDebug() << "array[" << i << "]: " << array.at(i);
		}
	}
	else {
		qWarning() << "Cant parse json: " << json;
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
				ChannelContainer ct(object);
				// TODO not much secure, need test all parameters
				ct.m_teamId = reply->property(P_TEAM_INDEX).toInt();
				ct.m_serverId = reply->property(P_SERVER_ID).toInt();
				m_server[ct.m_serverId].m_teams[ct.m_teamId].m_public_channels << ct;
				emit channelAdded(ct);
			}
		}
	}
}

void MattermostQt::reply_error(QNetworkReply *reply)
{
	QJsonDocument json = QJsonDocument::fromJson(reply->readAll());
	if( json.isObject() )
	{
		QJsonObject object = json.object();
		QString error_id = object["id"].toString();
		if( error_id.compare("api.user.check_user_password.invalid.app_error") == 0 )
		{
			emit connectionError(ConnectionError::WrongPassword, QObject::trUtf8("Login failed because of invalid password") );
		}
	}
}

void MattermostQt::replyFinished(QNetworkReply *reply)
{
	if (reply->error() == QNetworkReply::NoError) {
		//success

		QVariant replyType;
		replyType = reply->property(P_REPLY_TYPE);

		if(replyType.isValid())
		{
			switch (replyType.toInt()) {
			case ReplyType::Login:
				reply_login(reply);
				break;
			case ReplyType::Teams:
				reply_get_teams(reply);
				break;
			case ReplyType::Team:
				reply_get_public_channels(reply);
				break;
			default:
				qWarning() << "That can't be!";
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
		qDebug() << "Failure" <<reply->errorString();
//		qDebug() << "Reply: " << reply->readAll();
		reply_error(reply);
	}
	delete reply;
}

void MattermostQt::replySSLErrors(QNetworkReply *reply, QList<QSslError> errors)
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
		//case QSslError::CertificateUntrusted:
		case QSslError::SelfSignedCertificate:
		// TODO - remove HostNameMistchmach
		case QSslError::HostNameMismatch:
		{
			QVariant ts = reply->property(P_TRUST_CERTIFICATE);
			if( ts.isValid() && ts.toBool() )
				ignoreErrors << error;
			break;
		}
		default:
			break;
		}
	}
	reply->ignoreSslErrors(ignoreErrors);
}

void MattermostQt::onWebSocketConnected()
{
	QWebSocket * socket = qobject_cast<QWebSocket*>(sender());
	if(!socket) // strange situation, if it happens
		return;
	QVariant sId = socket->property(P_SERVER_ID);
	if(!sId.isValid()) // that too strange!!! that cant be!
		return;
	int id = sId.toInt();
	QMap<int,ServerContainer>::iterator it = m_server.find(id);
	if( it == m_server.end() ) // that really strange! whats wrong? how it could be?
		return;

//	ServerContainer sc = it.value();

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

//	QNetworkRequest req = socket->request();
//	req
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
//"update_at": 0,
//"delete_at": 0,
	m_team_id = object["team_id"].toString();
//"type": "string",
	m_display_name = object["display_name"].toString();
	m_name = object["name"].toString();
	m_header = object["header"].toString();
	m_purpose = object["purpose"].toString();
	m_last_post_at = (qlonglong)object["last_post_at"].toDouble();
	m_total_msg_count = (qlonglong)object["total_msg_count"].toDouble();
	m_extra_update_at = (qlonglong)object["extra_update_at"].toDouble();
	m_creator_id = object["creator_id"].toString();
}
