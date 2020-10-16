#include <QtQuick>
#include <QApplication>
#include <QDebug>
#include <TeamsModel.h>
#include <UsersModel.h>
#include <MattermostQt.h>
#include <ChannelsModel.h>
#include <MessagesModel.h>
#include <AccountsModel.h>
#include <AttachedFilesModel.h>
#include <EmojiModel.h>
#include <SettingsContainer.h>
#include "AppWindow.h"
#include "theme.h"
//#include "SailNotify.h"

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
	if( SettingsContainer::getInstance()->useLog() == false || SettingsContainer::getInstance()->logLevel() > (SettingsContainer::LogLevel)type )
		return; // TODO test no stdout
	// TODO write log to databse in separate thread
	QByteArray localMsg = msg.toLocal8Bit();
	switch (type) {
	case QtDebugMsg:
		fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
		break;
	case QtWarningMsg:
		fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
		break;
	case QtCriticalMsg:
		fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
		break;
	case QtFatalMsg:
		fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
		//abort();
	case QtInfoMsg:
		fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
		break;
	}
}

int main(int argc, char** argv)
{
	qInstallMessageHandler( myMessageOutput );

	QApplication app(argc, argv);

	QCoreApplication::setApplicationVersion(MATTERMOSTQT_VERSION);
	QCoreApplication::setOrganizationDomain("ru");
	QCoreApplication::setOrganizationName  ("sashikknox");
	QCoreApplication::setApplicationName   ("MostQute");

	qInfo() << "App version: " << MATTERMOSTQT_VERSION;

//	qmlRegisterType<MattermostQt>      ("ru.sashikknox", 1, 0, "MattermostQt");
	qmlRegisterType<TeamsModel>        ("ru.sashikknox", 1, 0, "TeamsModel");
	qmlRegisterType<UsersModel>        ("ru.sashikknox", 1, 0, "UsersModel");
	qmlRegisterType<ChannelsModel>     ("ru.sashikknox", 1, 0, "ChannelsModel");
	qmlRegisterType<MessagesModel>     ("ru.sashikknox", 1, 0, "MessagesModel");
	qmlRegisterType<ReverseProxyMessagesModel> ("ru.sashikknox", 1, 0, "ReverseProxyMessagesModel");
	qmlRegisterType<AccountsModel>     ("ru.sashikknox", 1, 0, "AccountsModel");
	qmlRegisterType<AttachedFilesModel>("ru.sashikknox", 1, 0, "AttachedFilesModel");
	qmlRegisterType<EmojiModel>        ("ru.sashikknox", 1, 0, "EmojiModel");
	qmlRegisterType<EmojiProxyCategory>("ru.sashikknox", 1, 0, "EmojiCategoryProxy");
	qmlRegisterType<EmojiProxyList    >("ru.sashikknox", 1, 0, "EmojiProxyList");
	qmlRegisterType<ChannelsFilterProxy>("ru.sashikknox", 1, 0, "ChannelsFilterProxy");
	qmlRegisterSingletonType<SettingsContainer>("ru.sashikknox", 1, 0, "Settings",     SettingsContainer_singletontype_provider );
	qmlRegisterSingletonType<MattermostQt>     ("ru.sashikknox", 1, 0, "MattermostQt", MattermostQt_singletontype_provider );
	qmlRegisterSingletonType<Theme>            ("ru.sashikknox", 1, 0, "Theme",        Theme_singletontype_provider );

	AppWindow mainWindow;
	mainWindow.show();

	return app.exec();
}
