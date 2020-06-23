#include "MessageEditArea.h"
#include <QKeyEvent>

MessageEditArea::MessageEditArea(QWidget *parent)
    : QTextEdit(parent)
{

}

void MessageEditArea::keyPressEvent(QKeyEvent *e)
{
	if( e->key() == Qt::Key_Return && e->modifiers() == Qt::NoModifier) {
		emit sendMessage(toPlainText());
	}
	else if( e->key() == Qt::Key_Return && e->modifiers() & Qt::ControlModifier ) {
		textCursor().insertText("\n");
	}
	else
		QTextEdit::keyPressEvent(e);
}
