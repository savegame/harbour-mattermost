#include "SettingsContainer.h"


MT_SET_PROPERTY(int,   autoDownloadImageSize)
MT_SET_PROPERTY(bool,  showBlobs)
MT_SET_PROPERTY(float, blobOpacity)


SettingsContainer::SettingsContainer(QObject *parent) : QObject(parent)
{
	resetToDefault();
}

void SettingsContainer::resetToDefault()
{
	m_autoDownloadImageSize = 512;
	m_showBlobs =false;
	m_blobOpacity = 1.0;
}
