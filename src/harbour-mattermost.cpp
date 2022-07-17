/*
  Copyright (C) 2022 ru.sashikknox.
  Contact: sashikknox <sashikknox@gmail.com>
  All rights reserved.

  You may use this file under the terms of MIT license

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
#include <QDebug>
#include <sailfishapp.h>
#include <Md4cParser.h>
#include "TeamsModel.h"
#include "MattermostQt.h"
#include "ChannelsModel.h"
#include "MessagesModel.h"
#include "AccountsModel.h"
#include "AttachedFilesModel.h"
#include "EmojiModel.h"
#include "SettingsContainer.h"
#include "SailNotify.h"

QFile *logFile = nullptr;

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
	if( SettingsContainer::getInstance()->useLog() == false || SettingsContainer::getInstance()->logLevel() > (SettingsContainer::LogLevel)type )
		return; // TODO test no stdout
	MattermostQt *m = MattermostQt::getInstance();
	// TODO write log to databse in separate thread
	QByteArray localMsg = msg.toLocal8Bit();
	switch (type) {
	case QtDebugMsg:
		fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
		if(logFile) {
			QTextStream out(logFile);
			out << QString("Debug: %0 (%1:%2, %3)\n").arg(msg).arg(context.file).arg(context.line).arg(context.function);
		}
		break;
	case QtWarningMsg:
		fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
		if(logFile) {
			QTextStream out(logFile);
			out << QString("Warning: %0 (%1:%2, %3)\n").arg(msg).arg(context.file).arg(context.line).arg(context.function);
		}
		break;
	case QtCriticalMsg:
		fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
		if(logFile) {
			QTextStream out(logFile);
			out << QString("Critical: %0 (%1:%2, %3)\n").arg(msg).arg(context.file).arg(context.line).arg(context.function);
		}
		break;
	case QtFatalMsg:
		fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
		if(logFile) {
			QTextStream out(logFile);
			out << QString("Fatal: %0 (%1:%2, %3)\n").arg(msg).arg(context.file).arg(context.line).arg(context.function);
		}
		abort();
		break;
	case QtInfoMsg:
		fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
		if(logFile) {
			QTextStream out(logFile);
			out << QString("Info: %0 (%1:%2, %3)\n").arg(msg).arg(context.file).arg(context.line).arg(context.function);
		}
		break;
	}
}

int main(int argc, char *argv[])
{
	// Set up QML engine.
	QScopedPointer<QGuiApplication> app(SailfishApp::application(argc, argv));

	QCoreApplication::setApplicationVersion(MATTERMOSTQT_VERSION);
//	QCoreApplication::setOrganizationDomain("harbour");
	QCoreApplication::setOrganizationName( QStringLiteral("ru.sashikknox") );
	QCoreApplication::setApplicationName( QStringLiteral("wattermost") );

	logFile = new QFile(SettingsContainer::getInstance()->logFilePath());
	if( !logFile->open(QIODevice::WriteOnly ) ) {
		delete logFile;
		logFile = nullptr;
	}
	qInstallMessageHandler( myMessageOutput );
	if( !logFile ) {
		qCritical() << QStringLiteral("Cant create log file: %0").arg(MattermostQt::getInstance()->settings()->logFilePath());
	}
	else {
		qInfo() << QStringLiteral("Create log file: %0").arg(logFile->fileName());
	}

	QScopedPointer<QQuickView> v(SailfishApp::createView());
	qInfo() << "App version: " << MATTERMOSTQT_VERSION;

	QScopedPointer<SailNotify> notify(new SailNotify());

	qmlRegisterType<TeamsModel>        ("ru.sashikknox", 1, 0, "TeamsModel");
	qmlRegisterType<ChannelsModel>     ("ru.sashikknox", 1, 0, "ChannelsModel");
	qmlRegisterType<MessagesModel>     ("ru.sashikknox", 1, 0, "MessagesModel");
	qmlRegisterType<AccountsModel>     ("ru.sashikknox", 1, 0, "AccountsModel");
	qmlRegisterType<AttachedFilesModel>("ru.sashikknox", 1, 0, "AttachedFilesModel");
	qmlRegisterType<EmojiModel>        ("ru.sashikknox", 1, 0, "EmojiModel");
	qmlRegisterType<EmojiProxyCategory>("ru.sashikknox", 1, 0, "EmojiCategoryProxy");
	qmlRegisterType<EmojiProxyList    >("ru.sashikknox", 1, 0, "EmojiProxyList");
	qmlRegisterType<ChannelsFilterProxy>("ru.sashikknox", 1, 0, "ChannelsFilterProxy");
	qmlRegisterSingletonType<SettingsContainer>("ru.sashikknox", 1, 0, "Settings", SettingsContainer_singletontype_provider );
	qmlRegisterSingletonType<MattermostQt>("ru.sashikknox", 1, 0, "MattermostQt", MattermostQt_singletontype_provider );

	// Start the application.
	v->setSource(SailfishApp::pathTo("qml/harbour-mattermost.qml"));
	v->show();

	MattermostQt *m = MattermostQt::getInstance();//v->rootObject()->findChild<MattermostQt*>();
//	SettingsContainer *s = v->rootObject()->findChild<SettingsContainer*>();
	if(m) {
		QObject::connect(m, &MattermostQt::newMessage, notify.data(), &SailNotify::slotNewMessage );
		m->replaceMdParser( new Md4cMdParser() );
	}
	else
		qCritical() << "Cant find MattermostQt main Object, possible you cant receive notifications!";
	QObject::connect(MattermostQt::getInstance(),&QObject::destroyed, [=](){
		if(logFile) {
			logFile->close();
			delete logFile;
			logFile = nullptr;
		}
	});
	return app->exec();
}
