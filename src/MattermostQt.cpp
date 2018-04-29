#include "MattermostQt.h"

#include <QNetworkRequest>
#include <QNetworkReply>

#include <QJsonDocument>
#include <QJsonObject>


#define P_REPLY_TYPE        "reply_type"
#define P_API               "api"
#define P_SERVER_URL        "server_url"
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

void MattermostQt::login(QString server, QString login, QString password, bool trustCertificate, int api)
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
	request.setRawHeader("X-Custom-User-Agent", "My app name v0.1");

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

			m_server[servers_count++] = newServer;
			emit serverConnected(servers_count - 1);
			// server get us authentificztion token, time to open WebSocket!
		}
	}
}

void MattermostQt::replyFinished(QNetworkReply *reply)
{
	if (reply->error() == QNetworkReply::NoError) {
		//success
		qDebug() << "Success" <<reply->readAll();

		QVariant replyType;
		replyType = reply->property(P_REPLY_TYPE);

		if(replyType.isValid())
		{
			switch (replyType.toInt()) {
			case ReplyType::Login:
				reply_login(reply);
				break;
			default:
				qWarning() << "That can't be!";
				break;
			}
		}
		else
		{
			qWarning() << "Unknown reply type";
		}
	}
	else {
		//failure
		qDebug() << "Failure" <<reply->errorString();
		qDebug() << "Reply: " << reply->readAll();
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
