#include "MattermostQt.h"

#include <QNetworkRequest>
#include <QNetworkReply>

MattermostQt::MattermostQt()
{
	m_networkManager.reset(new QNetworkAccessManager());

	connect(m_networkManager.data(), SIGNAL(finished(QNetworkReply*)), this,
	                 SLOT(replyFinished(QNetworkReply*)));
}

MattermostQt::~MattermostQt()
{

}

bool MattermostQt::login(QString server, QString login, QString password, int api)
{
	if(api <= 3)
		api = 4;
	// {"login_id":"someone@nowhere.com","password":"thisisabadpassword"}
	QString urlString = server + QLatin1String("/api/v")
	        + QString::number(api)
	        + QLatin1String("/users/login");
	// faster than QString("%0,%1").arg(arg0).arg(arg1);
	QString reqString = QLatin1String("{\"login_ia\":\"") +
	        login +
	        QLatin1String(",\"password\":\"")+
	        password +
	        QLatin1String("\"}");

	QUrl url(urlString);
	QNetworkRequest request;

	request.setUrl(url);
	request.setHeader(QNetworkRequest::ServerHeader, "application/json");
//	request.setAttribute( A);
	m_networkManager->post(request, reqString.toUtf8() );
}

void MattermostQt::replyFinished(QNetworkReply *reply)
{
	if( reply && reply->isReadable() )
	{
		QString ap = QString::fromUtf8(reply->readAll().data());
		qDebug() << ap ;
	}
}
