#include "AppWidget.h"
#include "ui_AppWidget.h"

#include <QtWebEngineWidgets>
#include <QWebEngineUrlRequestInterceptor>

#include <MattermostQt.h>
#include <TeamsModel.h>
#include <ChannelsModel.h>
#include <MessagesModel.h>
#include <SettingsContainer.h>
#include <views/MessageItemDelegate.h>
#include <views/TeamItemDelegate.h>
//#include <QtWebView>

class UrlCatcher : public QWebEngineUrlRequestInterceptor 
{
public:
	UrlCatcher(QObject *parent = nullptr) : QWebEngineUrlRequestInterceptor(parent)
	{
		
	}
	
	virtual void interceptRequest(QWebEngineUrlRequestInfo &info) 
	{	
		QString fullUrl = info.requestUrl().toString(QUrl::FullyEncoded);
		qWarning() << QStringLiteral("Itercept request: url(%0)").arg( fullUrl );
		if( fullUrl.contains("signup/gitlab/complete?") ) 
		{
			QRegExp re("(code|state)=([a-zA-Z0-9]+)&?");
			QString code;
			QString state;
			int lastPos = re.indexIn(fullUrl,0);
			while( lastPos != -1 )
			{
				if( re.cap(1) == QStringLiteral("code") ) {
					code = re.cap(2);
					lastPos += code.length();
				}
				else if( re.cap(1) == QStringLiteral("state") ) {
					state = re.cap(2);
					lastPos += state.length();
				}
				lastPos = re.indexIn(fullUrl,lastPos);
			}
			qWarning() << QStringLiteral("We authorized: code = %0; state %1;").arg(code).arg(state);
		}
	}
};

AppWidget::AppWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AppWidget)
{
	ui->setupUi(this);

//	ui->gitLabWebView
	//ui->pbServerGitLab->setVisible(false);
	ui->authenticationStack->setCurrentIndex(0);
	ui->pageStack->setCurrentIndex(1);

	TeamsModel *teamsModel = new TeamsModel();
	ChannelsModel *channelsModel = new ChannelsModel();
	m_channelsFilterProxy = new ChannelsFilterProxy(channelsModel);
	m_channelsFilterProxy->setSourceModel(channelsModel);
	teamsModel->setMattermostQt(MattermostQt::getInstance() );
	channelsModel->setMattermost( MattermostQt::getInstance() );
	ui->teamsView->setModel( teamsModel );
	ui->teamsView->setViewMode(QListView::IconMode);
	ui->teamsView->setItemDelegate( new TeamItemDelegate(teamsModel) );
	ui->channelsView->setModel(m_channelsFilterProxy);
	ui->channelsView->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->splitter->setStretchFactor(1,0);
	ui->splitter->setStretchFactor(3,1);
	connect(MattermostQt::getInstance(), &MattermostQt::serverStateChanged, this, &AppWidget::on_serverStateChanged );
	connect(MattermostQt::getInstance(), &MattermostQt::onConnectionError, this, &AppWidget::connectionError);
	connect(MattermostQt::getInstance(), &MattermostQt::teamAdded, this, &AppWidget::on_teamAdded);
	connect(ui->messageEdit, &MessageEditArea::sendMessage, this, &AppWidget::on_sendMesage );
	connect(this, &AppWidget::tokenChanged, this, &AppWidget::on_tokenChanged, Qt::QueuedConnection );
	// debuug 
//	ui->authenticationStack->setCurrentIndex(3);
}

AppWidget::~AppWidget()
{
	delete ui;
}

void AppWidget::setToken(QString tkn)
{
	emit tokenChanged(tkn);
}

void AppWidget::on_tokenChanged(QString token)
{
	this->token = token;
	MattermostQt::getInstance()->post_login_by_token(this->ui->leServerUrl->text(), token);
}

void AppWidget::connectionError(int code, QString message, int server_index)
{
	qCritical() << QStringLiteral("[%0] %1").arg(code).arg(message);
}

void AppWidget::on_cookieAdded(const QNetworkCookie &cookie)
{// Qt::QeuedConnection
	qWarning() << QStringLiteral("cookie: %0=%1;").arg(cookie.name().data()).arg(cookie.value().data());
	QString allCookie(cookie.toRawForm().data());
	if( allCookie.indexOf("MMAUTHTOKEN") != -1 ) {
		QRegExp authToken("MMAUTHTOKEN=([a-z0-9]+);");
		if( authToken.indexIn(allCookie) != -1 ) {
			this->setToken(authToken.cap(1));
			qCritical() << "Token is: " << token;
			QWidget *w = this->ui->authenticationStack->widget(3);
			for( auto c : w->children() ) {
				if( QWebEngineView* view = qobject_cast<QWebEngineView*>(c) ){
					view->deleteLater();
				}
			}
		}
	}
}

void AppWidget::on_teamAdded(MattermostQt::TeamPtr team)
{
	if( team->m_self_index == 0 )
	{
		ui->teamsView->setCurrentIndex(ui->teamsView->model()->index(0,0));
		on_teamsView_clicked(ui->teamsView->currentIndex());
		ui->teamsView->hide();
	}
}

void AppWidget::on_leServerUrl_textChanged(const QString &text)
{
//    if( ui->leServerUrl->text() )
	if( !text.isEmpty() )
		ui->pbServerNext->setEnabled(true);
	else
		ui->pbServerNext->setEnabled(false);
}

void AppWidget::on_pbServerNext_clicked()
{
	ui->authenticationStack->setCurrentIndex(1);
}

void AppWidget::on_pbServerGitLab_clicked()
{// GitLab authentication clicked
	ui->authenticationStack->setCurrentIndex(3);
}

void AppWidget::on_lePassword_textChanged(const QString &arg1)
{
	if( !ui->lePassword->text().isEmpty() && !ui->leUsername->text().isEmpty() )
		ui->pbLoginNext->setEnabled(true);
	else
		ui->pbLoginNext->setEnabled(false);
}

void AppWidget::on_leUsername_textChanged(const QString &arg1)
{
	if( !ui->lePassword->text().isEmpty() && !ui->leUsername->text().isEmpty() )
		ui->pbLoginNext->setEnabled(true);
	else
		ui->pbLoginNext->setEnabled(false);
}

void AppWidget::on_pbLoginNext_clicked()
{
	QString server = ui->leServerUrl->text();
	QString user = ui->leUsername->text();
	QString password = ui->lePassword->text();
	MattermostQt::getInstance()->post_login(server,user,password,4, QStringLiteral("Mattermost"), true);
}

void AppWidget::on_serverStateChanged(int server_index, int state)
{
	if( state == MattermostQt::ServerConnected )
	{
		ui->pageStack->setCurrentIndex(0);
		MattermostQt::getInstance()->get_teams(server_index);
	}
}

void AppWidget::on_teamsView_clicked(const QModelIndex &index)
{
	if( !index.isValid() )
		return;
	int server_index = 0;
//	int team_index = index.row();
//	QString team_id = MattermostQt::getInstance()->server()[server_index]->m_teams[team_index]->m_id;
	QString team_id = index.data(TeamsModel::TeamId).toString();
	current_team_index = index.row();
	MattermostQt::getInstance()->get_public_channels(server_index, team_id);
}

void AppWidget::on_channelsView_clicked(const QModelIndex &index)
{
	if(!index.isValid())
		return;

	int si = 0;
	int ti = index.data(ChannelsModel::TeamIndex).toInt();
	int ct = index.data(ChannelsModel::ChannelType).toInt();
	int ci = index.data(ChannelsModel::ChannelIndex).toInt();

	current_team_index = ti;
	current_channel_type = ct;
	current_channel_index = ci;
	ui->channelFilter->setText("");
//	m_channelsFilterProxy->setFilterRegExp("");

	//index.data(Qt::DecorationRole).value<QIcon>()
	if( ci >= 0 ) {
		ui->channelsView->selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Clear );
		ui->channelNameLabel->setText( index.data(ChannelsModel::DisplayName).toString() );
		ui->channelDescriptionLabel->setText( index.data(ChannelsModel::Header).toString() );
		ui->channelDescriptionLabel->setToolTip( index.data(ChannelsModel::RoleChannelDescription).toString() );
		ui->channelImageLabel->setPixmap( index.data(Qt::DecorationRole).value<QIcon>().pixmap( ui->channelImageLabel->maximumSize() ) );
		MattermostQt::getInstance()->get_posts(si,ti,ct,ci);
	}
}

void AppWidget::on_sendMesage(const QString &text)
{
	int si = 0;
	QString root_id = "";
//	ui->messagesView->model()->data( ui->messagesView->model()->index(0,0),  )
	MattermostQt::getInstance()->post_send_message(text, si, current_team_index, current_channel_type, current_channel_index, root_id);
	ui->messageEdit->clear();
}

void AppWidget::on_channelFilter_textChanged(const QString &text)
{
	m_channelsFilterProxy->setFilterRegExp(text);
}

void AppWidget::on_pbServerGitLab_clicked(bool checked)
{
	
}

void AppWidget::on_authenticationStack_currentChanged(int index)
{
	if( index == 3 ) // webview
	{
		QWidget *w = ui->authenticationStack->widget(3);
		if( w ) {
			QWebEngineView *webView = new QWebEngineView(w);
			QVBoxLayout *layout = new QVBoxLayout(w);
			layout->addWidget(webView);
			QWebEngineProfile::defaultProfile()->clearAllVisitedLinks();
			QWebEngineProfile::defaultProfile()->clearHttpCache();
			QWebEngineCookieStore *s = QWebEngineProfile::defaultProfile()->cookieStore();
			s->deleteAllCookies();
			s->deleteSessionCookies();
			connect(s, &QWebEngineCookieStore::cookieAdded, this, &AppWidget::on_cookieAdded, Qt::QueuedConnection);
	//		DEBUG_URL
	//		w->setLayout(layout);
			
	//		connect(webView, &QWebEngineView::titleChanged, [this, webView](const QString &title) {
	//			int index = indexOf(webView);
	//			if (index != -1) {
	//				setTabText(index, title);
	//				setTabToolTip(index, title);
	//			}
	//			if (currentIndex() == index)
	//				emit titleChanged(title);
	//		});
			connect(webView, &QWebEngineView::loadProgress, [this, webView](int progress) {
				this->ui->mainLabel->setText(QStringLiteral("%0\%").arg(progress) );
			});
			//QWebEngineView::urlChanged(const QUrl &url)
			connect(webView, &QWebEngineView::urlChanged, [this, webView](const QUrl &url) {
				qWarning() << QStringLiteral("Url changed: %0").arg(url.toString());
			});
			
			QWebEnginePage *page = new QWebEnginePage(webView);
#if QT_VERSION_MAJOR >= 5 && QT_VERSION_MINOR >= 13
			page->setUrlRequestInterceptor( new UrlCatcher(page) );
#endif
			connect(page, &QWebEnginePage::urlChanged, [this, page](const QUrl &url) {
//				page->
				qWarning() << QStringLiteral("Pages: Url changed: %0").arg(url.toString());
			});
//			page->setUrl(QUrl(DEBUG_URL));
			QUrl url(this->ui->leServerUrl->text());
			url.setPath( url.path() + QStringLiteral("/oauth/gitlab/login") );
			page->setUrl(url);
			webView->setPage(page);
		}
	}
}
