#include "AppWidget.h"
#include "ui_AppWidget.h"

#include <MattermostQt.h>
#include <TeamsModel.h>
#include <ChannelsModel.h>
#include <MessagesModel.h>
#include <SettingsContainer.h>
#include <views/MessageItemDelegate.h>
#include <views/TeamItemDelegate.h>
//#include <QtWebView>

AppWidget::AppWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AppWidget)
{
	ui->setupUi(this);

//	ui->gitLabWebView
	ui->pbServerGitLab->setVisible(false);
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
	ui->splitter->setStretchFactor(1,0);
	ui->splitter->setStretchFactor(3,1);
	connect(MattermostQt::getInstance(), &MattermostQt::serverStateChanged, this, &AppWidget::on_serverStateChanged );
	connect(ui->messageEdit, &MessageEditArea::sendMessage, this, &AppWidget::on_sendMesage );
}

AppWidget::~AppWidget()
{
	delete ui;
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
	m_channelsFilterProxy->setFilterRegExp("");

	//index.data(Qt::DecorationRole).value<QIcon>()
	if( ci >= 0 ) {
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
