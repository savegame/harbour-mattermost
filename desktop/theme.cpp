#include "theme.h"
#include <QDebug>


class ThemePrivate {
public:
	Theme::ColorScheme colorScheme;
	QColor primaryColor;
	QColor overlayBackgroundColor;
	qreal paddingSmall;
	qreal paddingMedium;
	qreal paddingLarge;
	int fontSizeSmall;
	int fontSizeMedium;
	int fontSizeLarge;
	qreal pixelRatio;
	qreal borderWidth;
};

Theme *Theme::getInstance()
{
	static Theme *theme = new Theme();
	bool first_call = true;
	if( first_call )
	{
		first_call = false;
		qInfo() << QStringLiteral("Theme::instance singleton object allocated;");
	}
	return theme;
}

Theme::Theme(QObject *parent)
    : QObject(parent)
{
	p = new ThemePrivate();
	p->colorScheme = ColorScheme::DarkOnLight;
	p->primaryColor = QColor(0,0,0);
	p->overlayBackgroundColor = QColor(255,255,255);
	p->pixelRatio = 3.14;
	p->borderWidth =  (1.0 < p->pixelRatio * 0.35) ?  p->pixelRatio * 0.35 : 1.0;
	p->paddingSmall  = p->pixelRatio * 2;
	p->paddingMedium = p->pixelRatio * 2.75;
	p->paddingLarge  = p->pixelRatio * 3.14;
	p->fontSizeSmall  = 12;
	p->fontSizeMedium = 16;
	p->fontSizeLarge  = 20;
}

Theme::~Theme()
{

}

qreal Theme::pixelRatio() const
{
	return p->pixelRatio;
}

Theme::ColorScheme Theme::colorScheme() const
{
	return p->colorScheme;
}

void Theme::setColorScheme(Theme::ColorScheme colorScheme)
{
	p->colorScheme = colorScheme;
	emit colorSchemeChanged();
}

qreal Theme::borderWidth() const
{
	return p->borderWidth;
}

int Theme::fontSizeSmall() const
{
	return p->fontSizeSmall;
}

int Theme::fontSizeMedium() const
{
	return p->fontSizeMedium;
}

int Theme::fontSizeLarge() const
{
	return p->fontSizeLarge;
}

QColor Theme::primaryColor() const
{
	return p->primaryColor;
}

QColor Theme::overlayBackgroundColor() const
{
	return p->overlayBackgroundColor;
}

qreal Theme::paddingSmall() const
{
	return p->paddingSmall;
}

qreal Theme::paddingMedium() const
{
	return p->paddingMedium;
}

qreal Theme::paddingLarge() const
{
	return p->paddingLarge;
}
