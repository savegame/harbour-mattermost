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
		qCritical() << QStringLiteral("Unable to set dark stylesheet, file not found");
	}
	else   {
		f.open(QFile::ReadOnly | QFile::Text);
		QTextStream ts(&f);
		qApp->setStyleSheet(ts.readAll());
		QStyle *s = qApp->style();

	}
	f.close();
}

//#include <o2.h>

//class TestGitLabO2 : O2 {
//public:
//	TestGitLabO2(QObject *parent = 0, QNetworkAccessManager *manager = 0, O0AbstractStore *store = 0)
//	    : O2(parent, manager, store)
//	{
//		// http://192.168.54.22/oauth/gitlab/login
//		setRequestUrl("http://192.168.54.22/oauth/gitlab/login");
//		setTokenUrl("http://192.168.54.22/oauth/gitlab/token");
//	}
//};

int main(int argc, char** argv)
{
	qInstallMessageHandler( myMessageOutput );

	QApplication app(argc, argv);
//	QtWebView::initialize();
	qApp->setAttribute(Qt::AA_EnableHighDpiScaling, true);
	qApp->setAttribute(Qt::AA_UseSoftwareOpenGL, true);
	QCoreApplication::setApplicationVersion(MATTERMOSTQT_VERSION);
	QCoreApplication::setOrganizationDomain("ru");
	QCoreApplication::setOrganizationName  ("sashikknox");
	QCoreApplication::setApplicationName   ("MostQute");

	qInfo() << "App version: " << MATTERMOSTQT_VERSION;

	loadStyle();
	
//	QNetworkAccessManager *nm = new QNetworkAccessManager();
//	TestGitLabO2 *oauth = new TestGitLabO2(nullptr, nm);

//	qDebug() << oauth->token();
	AppWidget mainWindow;
	mainWindow.show();

	return app.exec();
}
