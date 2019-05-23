#ifndef SETTINGSCONTAINER_H
#define SETTINGSCONTAINER_H

#include <QObject>
#include <QCoreApplication>
#include <QtQuick>
#include <QGradient>
#include "MattermostQt.h"

#define CONCAT1(a, b) a ## b
#define CONCAT2(a, b) CONCAT1(a, b)

#define CONCAT3(a, b, c) CONCAT2( CONCAT2(a, b), c )

#define MT_PROPERTY(type_, name_) \
	private: \
	    Q_PROPERTY( type_ name_ READ name_ WRITE CONCAT2(set_,name_) NOTIFY CONCAT3(on_, name_, Changed ) ) \
	    type_ CONCAT2(m_,name_); \
	public: \
	    type_ name_ () const \
        { \
	        return CONCAT2(m_,name_); \
	    } \
	    void CONCAT2(set_,name_) ( type_ value ); \
	Q_SIGNAL void CONCAT3(on_, name_, Changed) ()


#define MT_SET_PROPERTY(type_, name_) \
	void SettingsContainer:: CONCAT2(set_,name_) ( type_ value ) \
    { \
	    CONCAT2(m_,name_) = value; \
	    emit SettingsContainer:: CONCAT3(on_, name_, Changed) (); \
	    emit settingsChanged(); \
	} \


class SettingsContainer : public QObject
{
	Q_OBJECT

	/** download image automaticly, when its size lower than autoDownloadImageSize */
	MT_PROPERTY(int,   autoDownloadImageSize);
	MT_PROPERTY(bool,  showBlobs);
	MT_PROPERTY(float, blobOpacity);
	MT_PROPERTY(bool,  formatedText); // show Markdown text

public:
	explicit SettingsContainer(QObject *parent = nullptr);

	static SettingsContainer *getInstance();

	Q_INVOKABLE void resetToDefault();

Q_SIGNALS:
	void settingsChanged();
};

// Second, define the singleton type provider function (callback).
static QObject *SettingsContainer_singletontype_provider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
	Q_UNUSED(engine)
	Q_UNUSED(scriptEngine)
//	engine->
//	MattermostQt *m = engine->findChild<MattermostQt*>();
//	if(SettingsContainerSingleton.data())
//	SettingsContainer *settings = new SettingsContainer();
//	if(m)
//		m->setSettingsContainer(settings);
	return SettingsContainer::getInstance();
}

#endif // SETTINGSCONTAINER_H
