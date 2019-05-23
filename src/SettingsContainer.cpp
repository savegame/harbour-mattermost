#include "SettingsContainer.h"


MT_SET_PROPERTY(int,   autoDownloadImageSize)
MT_SET_PROPERTY(bool,  showBlobs)
MT_SET_PROPERTY(float, blobOpacity)
MT_SET_PROPERTY(bool, formatedText)


SettingsContainer::SettingsContainer(QObject *parent) : QObject(parent)
{
	resetToDefault();
}

SettingsContainer *SettingsContainer::getInstance()
{
	SettingsContainer *singleton = new SettingsContainer();
	return singleton;
}

void SettingsContainer::resetToDefault()
{
	m_autoDownloadImageSize = 512;
	m_showBlobs             = true;
	m_blobOpacity           = 0.7;
	m_formatedText          = true;
}
