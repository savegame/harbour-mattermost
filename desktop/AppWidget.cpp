#include "AppWidget.h"
#include "ui_AppWidget.h"

#include <MattermostQt.h>
#include <TeamsModel.h>
#include <ChannelsModel.h>
#include <MessagesModel.h>
#include <SettingsContainer.h>
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
	teamsModel->setMattermostQt(MattermostQt::getInstance());
	channelsModel->setMattermost( MattermostQt::getInstance());
	ui->teamsView->setModel( teamsModel );
	ui->channelsView->setModel(channelsModel);
	ui->teamsView->setViewMode(QListView::IconMode);
	connect(MattermostQt::getInstance(), &MattermostQt::serverStateChanged, this, &AppWidget::on_serverStateChanged );
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

void AppWidget::on_teamsView_activated(const QModelIndex &index)
{
	int server_index = 0;
	int team_index = index.row();
	MattermostQt::getInstance()->get_team(server_index, team_index);
}
