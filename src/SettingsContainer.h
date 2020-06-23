#ifndef SETTINGSCONTAINER_H
#define SETTINGSCONTAINER_H

#include <QObject>
#include <QCoreApplication>
#include <QtQuick>
#include <QString>
#include <QGradient>
#include "MattermostQt_gobal.h"
#include "MattermostQt.h"


class SettingsContainer : public QObject
{
	Q_OBJECT
public:
	enum Margin {
		MarginNone,
		MarginSmall,
		MarginMedium,
		MarginLarge,
		MarginDefault = MarginMedium,
	};
	Q_ENUM(Margin)

	enum ReactionSize {
		ReactionSmall,
		ReactionMedium,
		ReactionLarge,
		ReactionSizeDefault = ReactionMedium,
	};
	Q_ENUM(ReactionSize)

	enum LogLevel {
		LogLevelDebug = QtDebugMsg,
		LogLevelWarning = QtWarningMsg,
		LogLevelCritical = QtCriticalMsg,
		LogLevelFatal = QtFatalMsg,
		LogLevelInfo = QtInfoMsg,
#ifdef _DEBUG
		LogLevelDefault =  LogLevelDebug,
#else
		LogLevelDefault =  LogLevelFatal,
#endif
	};
	Q_ENUM(LogLevel)

	enum UserNameDisplay {
		UserNameLogin,      // just login name of user
		UserNameFull,       // try full user name (first and last name) or login, if it empty
		UserNameNickname,   // try use nickname or login, if it empty
		UserNameDisplayDefault = UserNameFull,
	};
	Q_ENUM(UserNameDisplay)

private:
	/** download image automaticly, when its size lower than autoDownloadImageSize */
	MT_PROPERTY(int,     autoDownloadImageSize);
	MT_PROPERTY(bool,    showBlobs);
	MT_PROPERTY(float,   blobOpacity);
	MT_PROPERTY(float,   pageMargin);
	MT_PROPERTY(int,     pageMarginEnum);
	MT_PROPERTY(bool,    formatedText); // show Markdown text
	MT_PROPERTY_DEF(bool,    debug, false);
	MT_PROPERTY(bool,    sendIcon); // ability change send message icon to icon-m-send from icon-m-mail
	MT_PROPERTY(ReactionSize, reactionSize); //set size of reactions
	MT_PROPERTY(QStringList,  usedReactions);
	MT_PROPERTY_DEF(int, usedReactionsCount, 21);
	MT_PROPERTY(bool,    searchInChannels); // show or not search field in channels view
	MT_PROPERTY(bool,    useCameraPicker);
	MT_PROPERTY_DEF(LogLevel, logLevel, LogLevelDefault);
	MT_PROPERTY_DEF(bool,     useLog, true);
	MT_PROPERTY_DEF(UserNameDisplay,  userNameDisplay, UserNameDisplayDefault);
public:
	explicit SettingsContainer(QObject *parent = nullptr);

	static SettingsContainer *getInstance();

	/**
	 * @brief strToSingleLine
	 * make one line string from multiline
	 * @param in multiline string
	 * @return  one line string
	 */
	Q_INVOKABLE static QString strToSingleLine(const QString &in);

	/**
	 * @brief resetToDefault
	 * reset settings to defalt values
	 */
	Q_INVOKABLE void resetToDefault();

	/**
	 * @brief asJsonObject
	 * write all settings to QJsonObject
	 * @return
	 */
	QJsonObject asJsonObject() const;

	/**
	 * @brief fromJsonObject
	 * read settings from json object
	 * @param object
	 */
	void fromJsonObject(const QJsonObject &settings);

	/**
	 * @brief addUsedReaction
	 * add reaction to list of used reactions
	 * @param reaction
	 */
	Q_INVOKABLE void addUsedReaction(const QString reaction);

private Q_SLOTS:
	void echoLogLevelChanged();
Q_SIGNALS:
	void settingsChanged();
};

// Second, define the singleton type provider function (callback).
static QObject *SettingsContainer_singletontype_provider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
	Q_UNUSED(engine)
	Q_UNUSED(scriptEngine)
	return SettingsContainer::getInstance();
}


#endif // SETTINGSCONTAINER_H
