#include <QDebug>

#include <QApplication>
//#include <QtWebView>
#include <QStyle>
#include <QDebug>
#include <TeamsModel.h>
#include <MattermostQt.h>
#include <ChannelsModel.h>
#include <MessagesModel.h>
#include <AccountsModel.h>
#include <AttachedFilesModel.h>
#include <EmojiModel.h>
#include <SettingsContainer.h>
#include <AppWidget.h>

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

void loadStyle() {
	QFile f(":qdarkstyle/style.qss");

	if (!f.exists())   {
		qCritical() << QStringLiteral("Unable to set stylesheet, file not found");
	}
	else   {
		f.open(QFile::ReadOnly | QFile::Text);
		QTextStream ts(&f);
		qApp->setStyleSheet(ts.readAll());
		QStyle *s = qApp->style();

	}
	f.close();
}

int main(int argc, char** argv)
{
	qInstallMessageHandler( myMessageOutput );

	QApplication app(argc, argv);
//	QtWebView::initialize();

	QCoreApplication::setApplicationVersion(MATTERMOSTQT_VERSION);
	QCoreApplication::setOrganizationDomain("ru");
	QCoreApplication::setOrganizationName  ("sashikknox");
	QCoreApplication::setApplicationName   ("MostQute");

	qInfo() << "App version: " << MATTERMOSTQT_VERSION;

	loadStyle();

	AppWidget mainWindow;
	mainWindow.show();

	return app.exec();
}
