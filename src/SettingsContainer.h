#ifndef SETTINGSCONTAINER_H
#define SETTINGSCONTAINER_H

#include <QObject>
#include <QCoreApplication>
#include <QtQuick>
#include <QGradient>

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
	} \


class SettingsContainer : public QObject
{
	Q_OBJECT

	/** download image automaticly, when its size lower than autoDownloadImageSize */
	MT_PROPERTY(int,   autoDownloadImageSize);
	MT_PROPERTY(bool,  showBlobs);
	MT_PROPERTY(float, blobOpacity);

public:
	explicit SettingsContainer(QObject *parent = nullptr);

	Q_INVOKABLE void resetToDefault();
};

// Second, define the singleton type provider function (callback).
static QObject *SeetingsContainer_singletontype_provider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
	Q_UNUSED(engine)
	Q_UNUSED(scriptEngine)

	SettingsContainer *settings = new SettingsContainer();
	return settings;
}

#endif // SETTINGSCONTAINER_H
