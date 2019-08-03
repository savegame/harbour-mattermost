#include "SettingsContainer.h"


MT_SET_PROPERTY(int,   autoDownloadImageSize)
//MT_SET_PROPERTY(bool,  showBlobs)
MT_SET_PROPERTY(float, blobOpacity)
MT_SET_PROPERTY(bool, formatedText)


void SettingsContainer::set_showBlobs ( bool value )
{
	m_showBlobs = value;
	emit SettingsContainer::on_showBlobsChanged();
	emit settingsChanged();
}

SettingsContainer::SettingsContainer(QObject *parent) : QObject(parent)
{
	resetToDefault();
}

SettingsContainer *SettingsContainer::getInstance()
{
	static SettingsContainer *singleton = new SettingsContainer();
	qDebug()<< "Get instance" << qlonglong(singleton);
	return singleton;
}

void SettingsContainer::resetToDefault()
{
	m_autoDownloadImageSize = 512;
	m_showBlobs             = true;
	m_blobOpacity           = 0.7;
	m_formatedText          = true;
}

#define ADD_VALUE(x) settings[#x] = x
#define FROM_VALUE(x,func) x = settings[#x].func

QJsonObject SettingsContainer::asJsonObject() const
{
	QJsonObject settings;
	ADD_VALUE(m_autoDownloadImageSize);
	ADD_VALUE(m_showBlobs);
	ADD_VALUE(m_blobOpacity);
	ADD_VALUE(m_formatedText);
	return settings;
}

void SettingsContainer::fromJsonObject(const QJsonObject &settings)
{
	FROM_VALUE(m_autoDownloadImageSize,toInt());
	FROM_VALUE(m_showBlobs,toBool());
	FROM_VALUE(m_blobOpacity,toDouble());
	FROM_VALUE(m_formatedText,toBool());
}
