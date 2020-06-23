#ifndef MESSAGEEDITAREA_H
#define MESSAGEEDITAREA_H

#include <QTextEdit>

class MessageEditArea : public QTextEdit
{
	Q_OBJECT
public:
	explicit MessageEditArea(QWidget *parent = nullptr);

protected:
	virtual void keyPressEvent(QKeyEvent *e) override;

signals:
	void sendMessage(const QString &text);

public slots:
};

#endif // MESSAGEEDITAREA_H
