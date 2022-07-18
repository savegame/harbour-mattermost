#include "SettingsContainer.h"

#include <QJsonObject>
#include <QJsonArray>

#ifdef SAILFISHAPP
#include <sailfishapp.h>
#include <libsailfishsilica/silicatheme.h>
#endif
#include <QStandardPaths>

class SettingsContainerPrivate {
friend class SettingsContainer;
};

#define SET_SETTINGS_PROPERTY(_t, _n) MT_SET_PROPERTY_2(SettingsContainer, _t, _n, settingsChanged)
// TODO make two difenes for RW properties and R only properties
SET_SETTINGS_PROPERTY(int,               autoDownloadImageSize)
SET_SETTINGS_PROPERTY(bool,              showBlobs)
SET_SETTINGS_PROPERTY(float,             blobOpacity)
SET_SETTINGS_PROPERTY(float,             pageMargin)
SET_SETTINGS_PROPERTY(int,               pageMarginEnum)
SET_SETTINGS_PROPERTY(bool,              formatedText)
SET_SETTINGS_PROPERTY(bool,              debug)
SET_SETTINGS_PROPERTY(bool,              sendIcon)
SET_SETTINGS_PROPERTY(QStringList,       usedReactions)
SET_SETTINGS_PROPERTY(int,               usedReactionsCount)
SET_SETTINGS_PROPERTY(bool,              searchInChannels)
SET_SETTINGS_PROPERTY(bool,              useCameraPicker)
SET_SETTINGS_PROPERTY(bool,              useLog)
SET_SETTINGS_PROPERTY(LogLevel,          logLevel)
SET_SETTINGS_PROPERTY(ReactionSize,      reactionSize)
SET_SETTINGS_PROPERTY(UserNameDisplay,   userNameDisplay)
SET_SETTINGS_PROPERTY(QString,           logFilePath)

QString getLogFileLocationDefault()
{
	return QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + QDir::separator() + QStringLiteral("mattermost_0.log");
}

SettingsContainer::SettingsContainer(QObject *parent) : QObject(parent)
{
	connect(this, &SettingsContainer::logLevelChanged, this, &SettingsContainer::echoLogLevelChanged );
	resetToDefault();
}

SettingsContainer *SettingsContainer::	getInstance()
{
	static SettingsContainer *singleton = new SettingsContainer();
//	if(singleton->debug())
//		qDebug()<< "Get instance" << qlonglong(singleton);
	return singleton;
}

QString SettingsContainer::strToSingleLine(const QString &in)
{
	QString r = in;
	r= r.replace(QRegExp("(\\n|\\r\\n)")," ");
	return r;
}

void SettingsContainer::resetToDefault()
{
#ifdef SAILFISHAPP
	m_pageMargin            = Silica::Theme::instance()->paddingMedium();
#else
	m_pageMargin            = 12; // TODO pageMargin for other platforms
#endif
	m_autoDownloadImageSize = 512;
	m_showBlobs             = true;
	m_blobOpacity           = 0.7;
	m_formatedText          = true;
	m_pageMarginEnum        = SettingsContainer::MarginMedium;
	m_sendIcon              = true;
	m_searchInChannels      = true;
	m_useCameraPicker       = true;
	m_useLog                = true;
	m_logLevel              = LogLevelDefault;
	m_userNameDisplay       = UserNameDisplayDefault;
	m_logFilePath           = getLogFileLocationDefault();
}

#define ADD_VALUE(x) settings[#x] = x
#define ADD_ENUM(x, _enum)  settings[#x] = QVariant::fromValue<_enum>(x).toString()
#define FROM_VALUE(x,func) x = settings[#x].func
#define FROM_ENUM(x, _enum) x = QVariant(settings[#x].toString()).value<_enum>()
#define ADD_LIST(x) {\
	    QJsonArray array;\
	    for(auto v : x) {\
	        array.push_back(v);\
	    }\
	    settings[#x] = array; \
	}

#define FROM_LIST(x) {\
	    x.clear(); \
	    QJsonArray array = settings[#x].toArray();\
	    for(auto v : array) {\
	        x.push_back( v.toString() ); \
	    }\
	}
// _enum ::CONCAT2(_enum,Default)
QJsonObject SettingsContainer::asJsonObject() const
{
	QJsonObject settings;
	ADD_VALUE(m_autoDownloadImageSize);
	ADD_VALUE(m_showBlobs);
	ADD_VALUE(m_blobOpacity);
	ADD_VALUE(m_formatedText);
	ADD_VALUE(m_pageMarginEnum);
	ADD_VALUE(m_sendIcon); // if false, use old icon-m-mail
	ADD_ENUM(m_reactionSize, ReactionSize);
	ADD_LIST(m_usedReactions);
	ADD_VALUE(m_usedReactionsCount);
	ADD_VALUE(m_searchInChannels);
	ADD_VALUE(m_useCameraPicker);
//	ADD_VALUE(m_debug);
	ADD_ENUM(m_logLevel, LogLevel);
	ADD_VALUE(m_useLog);
	ADD_ENUM(m_userNameDisplay, UserNameDisplay);
	ADD_VALUE(m_logFilePath);
	return settings;
}

void SettingsContainer::fromJsonObject(const QJsonObject &settings)
{
	FROM_VALUE(m_autoDownloadImageSize,toInt());
	FROM_VALUE(m_showBlobs,toBool());
	FROM_VALUE(m_blobOpacity,toDouble());
	FROM_VALUE(m_formatedText,toBool());
	FROM_VALUE(m_pageMarginEnum,toInt());
	FROM_VALUE(m_sendIcon  ,toBool());
	FROM_ENUM (m_reactionSize, ReactionSize);
	FROM_LIST (m_usedReactions);
	FROM_VALUE(m_usedReactionsCount, toInt());
	FROM_VALUE(m_searchInChannels, toBool());
	FROM_VALUE(m_useCameraPicker, toBool());
//	FROM_VALUE(m_debug, toBool());
	FROM_ENUM(m_logLevel, LogLevel);
	FROM_VALUE(m_useLog, toBool());
	FROM_ENUM(m_userNameDisplay, UserNameDisplay);
	FROM_VALUE(m_logFilePath, toString(getLogFileLocationDefault()));

	if( m_usedReactionsCount == 0 )
		m_usedReactionsCount = 21;

	switch(m_pageMarginEnum) {
	case Margin::MarginNone:
		m_pageMargin = 0;
		break;
	case Margin::MarginSmall:
#ifdef SAILFISHAPP
		m_pageMargin = Silica::Theme::instance()->paddingSmall();
#else
		m_pageMargin = 10;// TODO pageMargin for other platforms
#endif
		break;
	case Margin::MarginMedium:
#ifdef SAILFISHAPP
		m_pageMargin = Silica::Theme::instance()->paddingMedium();
#else
		m_pageMargin = 14;// TODO pageMargin for other platforms
#endif
		break;
	case Margin::MarginLarge:
#ifdef SAILFISHAPP
		m_pageMargin = Silica::Theme::instance()->paddingLarge();
#else
		m_pageMargin = 18;// TODO pageMargin for other platforms
#endif
		break;
	}
}

void SettingsContainer::addUsedReaction(const QString reaction)
{
	int index = m_usedReactions.indexOf(reaction);
	if( index != -1 )
	{ // than move this reaction to the front
		m_usedReactions.move( index, 0 );
	}
	else
	{
		if( m_usedReactions.size() >= m_usedReactionsCount)
			m_usedReactions.pop_back();
		m_usedReactions.push_front(reaction);
	}
	emit usedReactionsChanged();
	emit settingsChanged();
}

void SettingsContainer::echoLogLevelChanged()
{
	qInfo() << QStringLiteral("Log level changed to: %0").arg( QVariant::fromValue<LogLevel>(m_logLevel).toString() );
}
