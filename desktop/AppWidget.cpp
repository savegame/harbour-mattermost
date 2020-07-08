#include "AppWidget.h"
#include "ui_AppWidget.h"

#include <QtWebEngineWidgets>
#include <QWebEngineUrlRequestInterceptor>
#include <QTimer>

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
	ui->attachCountLabel->hide(); // hidden while no attaches

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
	connect(MattermostQt::getInstance(), &MattermostQt::fileUploadProgress, this, &AppWidget::on_fileUploadProgress);
	connect(MattermostQt::getInstance(), &MattermostQt::fileUploaded, this, &AppWidget::on_fileUploaded);
	connect(ui->messageEdit, &MessageEditArea::sendMessage, this, &AppWidget::on_sendMesage );
	connect(this, &AppWidget::tokenChanged, this, &AppWidget::on_tokenChanged, Qt::QueuedConnection );
	// debuug 
//	ui->authenticationStack->setCurrentIndex(3);
	// setup timer
	QTimer *timer = new QTimer(this);
	connect(timer, &QTimer::timeout, this, QOverload<>::of(&AppWidget::on_timeUpdate));
	timer->start(1000);
}

AppWidget::~AppWidget()
{
	delete ui;
}

void AppWidget::setToken(QString tkn)
{
	emit tokenChanged(tkn);
}

void AppWidget::mouseMoveEvent(QMouseEvent *e)
{
	QWidget::mouseMoveEvent(e);
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
	if( !text.isEmpty() || !m_attaches.empty() ) {
		MattermostQt::getInstance()->post_send_message(text, si, current_team_index, current_channel_type, current_channel_index, root_id);
		m_attaches.clear();
		ui->attachCountLabel->setText( QLatin1String("0") );
		ui->attachCountLabel->setToolTip( QStringLiteral("%0 %1").arg(tr("Attached files count is")).arg(ui->attachCountLabel->text()) );
		ui->attachCountLabel->hide();
//		if( ui->attachmentsArea->layout() )
//		for(int i = 0; i < ui->attachmentsArea->layout()->count(); i++) {
//			if( QProgressBar *pb = qobject_cast<QProgressBar*>(ui->attachmentsArea->layout()->itemAt(i)->widget()) ) {
//				pb->deleteLater();
//			}
//		}
//		ui->attachmentsArea->hide();
	}
	ui->messageEdit->clear();
}

void AppWidget::on_channelFilter_textChanged(const QString &text)
{
	m_channelsFilterProxy->setFilterRegExp(text);
}

void AppWidget::on_authenticationStack_currentChanged(int index)
{
	if( index == 3 ) {// webview
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
			connect(webView, &QWebEngineView::loadProgress, [this](int progress) {
				this->ui->mainLabel->setText(QStringLiteral("%0 %").arg(progress) );
			});
			//QWebEngineView::urlChanged(const QUrl &url)
			connect(webView, &QWebEngineView::urlChanged, [](const QUrl &url) {
				qWarning() << QStringLiteral("Url changed: %0").arg(url.toString());
			});
			
			QWebEnginePage *page = new QWebEnginePage(webView);
#if QT_VERSION_MAJOR >= 5 && QT_VERSION_MINOR >= 13
			page->setUrlRequestInterceptor( new UrlCatcher(page) );
#endif
			connect(page, &QWebEnginePage::urlChanged, [](const QUrl &url) {
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

void AppWidget::on_fileUploadProgress(QString data, int progress)
{
	QStringList params = data.split("; ");
	QString channel_id = params.front();
	QString file_path = params.back();
//	QList<AttachmentPtr>::iterator search = std::find_if( m_attaches.begin(), m_attaches.end(), [file_path](const AttachmentPtr &a) -> bool {
//		if( a->path == file_path )
//			return true;
//		return false;
//	});
//	int progress
//	if( search != m_attaches.end() ) {
//		(*search)->progress = progress;
//		for(int i = 0; i < ui->attachmentsArea->layout()->count(); i++) {
//			if( QProgressBar *pb = qobject_cast<QProgressBar*>(ui->attachmentsArea->layout()->itemAt(i)->widget()) ) {
//				if( pb->property("P_FILE_PATH").toString() == file_path ) {
//					pb->setValue( int(progress * 100) );
//					break;
//				}
//			}
//		}
//	}
//	ui->attachmentsArea->show();
	float all_progress = 0.0;
	int attaches_count = m_attaches.count();
	for( AttachmentPtr a : m_attaches )
	{
		if( a->path == file_path )
		{
			a->progress = progress;
		}
		all_progress += a->progress;
	}
	all_progress = all_progress / attaches_count;

	ui->infoLabel->setText( QStringLiteral("%0 %1\%").arg(tr("Uploading")).arg( int(all_progress) ) );
}

void AppWidget::on_fileUploaded(int server_index, int file_sc_index)
{
	MattermostQt::FilePtr file = MattermostQt::getInstance()->server()[server_index]->m_file[file_sc_index];
//	for(int i = 0; i < ui->attachmentsArea->layout()->count(); i++) {
//		if( QProgressBar *pb = qobject_cast<QProgressBar*>(ui->attachmentsArea->layout()->itemAt(i)->widget()) ) {
//			if( pb->property("P_FILE_PATH").toString() == file->m_file_path ) {
//				pb->setValue( 100 );
//				break;
//			}
//		}
//	}
	bool all_finished = true;
	int finished_count = 0;
	for( AttachmentPtr a : m_attaches )
	{
		if( a->path == file->m_file_path )
		{
			a->progress = 100;
		}
		if( a->progress != 100 )
		{
			all_finished = false;
		}
		else
			finished_count += 1;
	}
	if( finished_count > 0 ) {
		ui->attachCountLabel->show();
		ui->attachCountLabel->setText( QString::number(finished_count) );
	}
	if(all_finished)
	{
		for( AttachmentPtr a : m_attaches )
		{
			a->finished = true;
		}
	}
}

void AppWidget::on_attachButton_clicked()
{
	QStringList files = QFileDialog::getOpenFileNames(this, tr("Caption"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)/*SettingsContainer::getInstance()->lastOpenFolder()*/);

	for( auto file : files ) {
		MattermostQt::getInstance()->post_file_upload(current_server_index, current_team_index, current_channel_type, current_channel_index, file);
		m_attaches.append( AttachmentPtr( new Attachment(file,MattermostQt::FileUnknown)) );
//		QProgressBar *pb = new QProgressBar(ui->attachmentsArea);
//		pb->setMaximumSize( QSize(96,96) );
//		pb->setMinimum(0);
//		pb->setMaximum(100);
//		pb->setProperty("P_FILE_PATH", file);
//		if( !ui->attachmentsArea->layout() )
//		{
//			QHBoxLayout *l = new QHBoxLayout(ui->attachmentsArea);
//			ui->attachmentsArea->setLayout(l);
//		}
//		ui->attachmentsArea->layout()->addWidget(pb);
	}
}

void AppWidget::on_timeUpdate()
{
	// chek for unfihished attachments
	for( AttachmentPtr a : m_attaches )
	{
		if( a->finished == false ) {
			break;
		}
	}

	ui->infoLabel->setText( QDateTime::currentDateTime().toString("hh:mm") );
}
