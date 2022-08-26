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

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
	if( SettingsContainer::getInstance()->useLog() == false || SettingsContainer::getInstance()->logLevel() > (SettingsContainer::LogLevel)type )
		return; // TODO test no stdout

	static QScopedPointer<QFile> logFile( new QFile(SettingsContainer::getInstance()->logFilePath()) );
	static QScopedPointer<QTextStream> out; //(logFile.data());
	static bool use_log_file = false;

	// TODO write log to databse in separate thread
	if( use_log_file == false ) {
		if( logFile->open(QIODevice::WriteOnly ) ) {
			out.reset( new QTextStream(logFile.data()) );
		} else {
			logFile.reset(nullptr);
			fprintf(stderr, "E: Cant init log file\n");
		}
		use_log_file = true;
	}

	QByteArray localMsg = msg.toLocal8Bit();
	char prefix = 'U';
	switch (type) {
	case QtDebugMsg:
		prefix = 'D';
		break;
	case QtWarningMsg:
		prefix = 'W';
		break;
	case QtCriticalMsg:
		prefix = 'E';
		break;
	case QtFatalMsg:
		prefix = 'F';
		break;
	case QtInfoMsg:
		prefix = 'I';
		break;
	}

	fprintf(stderr, "%c: %s (%s:%u, %s)\n", prefix, localMsg.constData(), context.file, context.line, context.function);
	if(out) {
		*out.data() << QString("%4: %0 (%1:%2, %3)\n").arg(msg).arg(context.file).arg(context.line).arg(context.function).arg(prefix);
	}
	if( type == QtFatalMsg ) {
		abort();
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

	qInstallMessageHandler( myMessageOutput );

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

	QObject::connect(MattermostQt::getInstance(), &MattermostQt::newMessage, notify.data(), &SailNotify::slotNewMessage );
	MattermostQt::getInstance()->replaceMdParser( new Md4cMdParser() );

	return app->exec();
}
