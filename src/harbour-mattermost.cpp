/*
  Copyright (C) 2013 Jolla Ltd.
  Contact: Thomas Perl <thomas.perl@jollamobile.com>
  All rights reserved.

  You may use this file under the terms of BSD license as follows:

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
	* Redistributions of source code must retain the above copyright
	  notice, this list of conditions and the following disclaimer.
	* Redistributions in binary form must reproduce the above copyright
	  notice, this list of conditions and the following disclaimer in the
	  documentation and/or other materials provided with the distribution.
	* Neither the name of the Jolla Ltd nor the
	  names of its contributors may be used to endorse or promote products
	  derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR
  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <QtQuick>
#include <QCoreApplication>
#include <sailfishapp.h>
#include "TeamsModel.h"
#include "MattermostQt.h"
#include "ChannelsModel.h"
#include "MessagesModel.h"
#include "SailNotify.h"

int main(int argc, char *argv[])
{
	// Set up QML engine.
	QScopedPointer<QGuiApplication> app(SailfishApp::application(argc, argv));
	QScopedPointer<QQuickView> v(SailfishApp::createView());
	QScopedPointer<SailNotify> notify(new SailNotify());

	QCoreApplication::setApplicationVersion(MATTERMOSTQT_VERSION);
	QCoreApplication::setOrganizationDomain("harbour");
	QCoreApplication::setOrganizationName("sashikknox");

	// If you wish to publish your app on the Jolla harbour, it is recommended
	// that you prefix your internal namespaces with "harbour.".
	//
	// For details see:
	// https://harbour.jolla.com/faq#1.5.0
	qmlRegisterType<MattermostQt>("harbour.sashikknox", 1, 0, "MattermostQt");
	qmlRegisterType<TeamsModel>("harbour.sashikknox", 1, 0, "TeamsModel");
	qmlRegisterType<ChannelsModel>("harbour.sashikknox", 1, 0, "ChannelsModel");
	qmlRegisterType<MessagesModel>("harbour.sashikknox", 1, 0, "MessagesModel");

	// Start the application.
	v->setSource(SailfishApp::pathTo("qml/harbour-mattermost.qml"));
	v->show();

////	QObject *item = dynamic_cast<QObject *>(v->engine()->rootContext()->baseUrl()rootObjects().at(0));
//	foreach(QObject *object , v->engine()->children() )
//	{
//		qDebug() << QQmlProperty::read(object, "context").toString();
//		foreach(QObject *object2 , object->children() )
//			qDebug() << "  " << QQmlProperty::read(object2, "name").toString();
//	}

//	v->engine()
//	QVariant value;
//	QMetaObject::invokeMethod(v->engine()->findChild<QObject*>("harbour-mattermost"), "getMattermost", Qt::DirectConnection,
//	                          Q_RETURN_ARG(QVariant, value) );
//	qWarning() << value;

	MattermostQt *m = v->rootObject()->findChild<MattermostQt*>();
	if(m)
		QObject::connect(m, SIGNAL(newMessage(QString,QString)), notify.data(), SLOT(slotNewMessage(QString,QString)) );

	return app->exec();
}
