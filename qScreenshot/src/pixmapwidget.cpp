/*
 * pixmapwidget.cpp
 * Copyright (C) 2009-2011  Khryukin Evgeny
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include <QtGui>

#include "pixmapwidget.h"
#include "options.h"
#include "defines.h"
#include "iconset.h"

#define ACCURACY 5

//------------------------------
//---GetTextDlg-----------------
//------------------------------
class GetTextDlg : public QDialog
{
	Q_OBJECT
public:
	GetTextDlg(QWidget *parent)
		: QDialog(parent)
	{
		setWindowTitle(tr("Enter text"));
		QVBoxLayout *l = new QVBoxLayout(this);
		QHBoxLayout *boxLayout = new QHBoxLayout;
		QPushButton *selectFont = new QPushButton(tr("Select Font"));
		selectFont->setIcon(Iconset::instance()->getIcon(QStyle::SP_MessageBoxQuestion));
		QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
							     Qt::Horizontal, this);

		boxLayout->addWidget(selectFont);
		boxLayout->addStretch();
		boxLayout->addWidget(box);

		te = new QTextEdit();

		l->addWidget(te);
		l->addLayout(boxLayout);

		connect(box, SIGNAL(accepted()), SLOT(okPressed()));
		connect(box, SIGNAL(rejected()), SLOT(close()));
		connect(selectFont, SIGNAL(released()),SIGNAL(selectFont()));

		adjustSize();
		setFixedSize(size());
		te->setFocus();
	}

signals:
	void text(const QString&);
	void selectFont();

private slots:
	void okPressed()
	{
		emit text(te->toPlainText());
		close();
	}

private:
	QTextEdit *te;
};




//------------------------------
//---SelectionRect--------------
//------------------------------
SelectionRect::SelectionRect()
	: QRect()
{
}

SelectionRect::SelectionRect(int left, int top, int w, int h)
	: QRect(left, top, w, h)
{
}

void SelectionRect::clear()
{
	setTop(-1);
	setLeft(-1);
	setHeight(-1);
	setWidth(-1);
}

SelectionRect::CornerType SelectionRect::cornerUnderMouse(QPoint pos) const
{
	if(!isValid()) {
		return NoCorner;
	}
	if(qAbs(topLeft().x() - pos.x()) < ACCURACY
	   && qAbs(topLeft().y() - pos.y()) < ACCURACY) {
		return TopLeft;
	}
	else if(qAbs(topRight().x() - pos.x()) < ACCURACY
		&& qAbs(topRight().y() - pos.y()) < ACCURACY) {
		return TopRight;
	}
	else if(qAbs(bottomLeft().x() - pos.x()) < ACCURACY
		&& qAbs(bottomLeft().y() - pos.y()) < ACCURACY) {
		return BottomLeft;
	}
	else if(qAbs(bottomRight().x() - pos.x()) < ACCURACY
		&& qAbs(bottomRight().y() - pos.y()) < ACCURACY) {
		return BottomRight;
	}

	return NoCorner;
}



//--------------------------------------------
//---------------PixmapWidget-----------------
//--------------------------------------------
PixmapWidget::PixmapWidget(QWidget *parent)
	: QWidget(parent)
	, bar_(0)
	, type_(ToolBar::ButtonNoButton)
	, p1(QPoint(-1, -1))
	, p2(QPoint(-1, -1))
	, selectionRect(new SelectionRect(-1,-1,-1,-1))
	, currentCursor(QCursor(Qt::CrossCursor))
	, cornerType(SelectionRect::NoCorner)
	, smoothLineType_(None)
{
	draftPen.setColor(Qt::red);
	draftPen.setStyle(Qt::DashLine);
	draftPen.setWidth(1);

	setMouseTracking(true);
}

PixmapWidget::~PixmapWidget()
{
	delete selectionRect;
}

void PixmapWidget::init(int lineWidth, const QString &color, const QString &font)
{
	color_ = QColor(color);
	font_.fromString(font);
	pen.setColor(color_);
	pen.setStyle(Qt::SolidLine);
	pen.setWidth(lineWidth);

	bar_->setColorForColorButton(color_);
	bar_->setLineWidth(lineWidth);
}

void PixmapWidget::setToolBar(ToolBar *bar)
{
	bar_ = bar;
	connect(bar_,SIGNAL(buttonClicked(ToolBar::ButtonType)), this, SLOT(buttonClicked(ToolBar::ButtonType)));
	connect(bar_,SIGNAL(checkedButtonChanged(ToolBar::ButtonType)),this, SLOT(checkedButtonChanged(ToolBar::ButtonType)));
	connect(bar_,SIGNAL(newWidth(int)),this,SLOT(newWidth(int)));
	bar_->checkButton(ToolBar::ButtonSelect);

	Options* o = Options::instance();
	init(o->getOption(constPenWidth, 2).toInt(),
	     o->getOption(constColor, "#fffff").toString(),
	     o->getOption(constFont, "Sans Serif,18,-1,5,50,0,0,0,0,0").toString());
}

void PixmapWidget::buttonClicked(ToolBar::ButtonType t)
{
	switch(t) {
	case ToolBar::ButtonCut:
		cut();
		return;
	case ToolBar::ButtonRotate:
		rotate();
		return;
	case ToolBar::ButtonColor:
		selectColor();
		return;
	case ToolBar::ButtonUndo:
		undo();
		return;
	case ToolBar::ButtonCopy:
		copy();
		return;
	case ToolBar::ButtonInsert:
		insert();
		return;
	default:
		break;
	}

	selectionRect->clear();
	update();
}

void PixmapWidget::newWidth(int w)
{
	pen.setWidth(w);
	settingsChanged(constPenWidth, QVariant(w));
}

void PixmapWidget::setPixmap(const QPixmap& pix)
{
	mainPixmap = pix;	
	setFixedSize(mainPixmap.size());
	selectionRect->clear();
	update();
}

void PixmapWidget::cut()
{
	if(selectionRect->x() == -1)
		return;

	saveUndoPixmap();
	setPixmap(mainPixmap.copy((QRect)*selectionRect));
	emit adjusted();
}

void PixmapWidget::insert()
{
	const QPixmap pix = qApp->clipboard()->pixmap();
	if(!pix.isNull()) {
		saveUndoPixmap();
		setPixmap(pix);
		emit adjusted();
	}
}

void PixmapWidget::copy()
{
	QClipboard *clipboard = QApplication::clipboard();
	QPixmap pix;
	if(selectionRect->width() != -1) {
		pix = mainPixmap.copy((QRect)*selectionRect);
	}
	else {
		pix = mainPixmap;
	}

	clipboard->setPixmap(pix);
}

void PixmapWidget::rotate()
{
	saveUndoPixmap();
	QTransform t;
	setPixmap(mainPixmap.transformed(t.rotate(90), Qt::SmoothTransformation));
	emit adjusted();
}

void PixmapWidget::paintEvent(QPaintEvent *)
{
	QPainter p(this);
	p.setClipRect(rect());
	p.drawPixmap(QPoint(0, 0), mainPixmap/*.scaled(size(), Qt::KeepAspectRatio, Qt::SmoothTransformation)*/);
	if(p2.x() != -1) {
		int w = (p2.x() - p1.x());
		int h = (p2.y() - p1.y());
		if(type_ == ToolBar::ButtonSelect || type_ == ToolBar::ButtonText) {
			p.setPen(draftPen);
			p.drawRect(p1.x(), p1.y(), w, h);
		}
		else if(type_ == ToolBar::ButtonEllipse) {
			p.setPen(pen);
			p.drawEllipse(p1.x(), p1.y(), w, h);
		}
		else if(type_ == ToolBar::ButtonRect) {
			p.setPen(pen);
			p.drawRect(p1.x(), p1.y(), w, h);
		}
	}
}

void PixmapWidget::mousePressEvent(QMouseEvent *e)
{
	if(e->buttons() == Qt::LeftButton) {
		if(cornerType == SelectionRect::NoCorner) {
			p1 = e->pos();
			p2 = QPoint(-1, -1);
			if(type_ == ToolBar::ButtonPen) {
				smoothLineType_ = None;
				setAttribute(Qt::WA_OpaquePaintEvent, true);
				p2 = p1;
				saveUndoPixmap();
				paintToPixmap();
			}
		}
		else if(cornerType == SelectionRect::TopLeft) {
			p1 = selectionRect->bottomRight();
			p2 = selectionRect->topLeft();
		}
		else if(cornerType == SelectionRect::TopRight) {
			p1 = selectionRect->bottomLeft();
			p2 = selectionRect->topRight();
		}
		else if(cornerType == SelectionRect::BottomLeft) {
			p1 = selectionRect->topRight();
			p2 = selectionRect->bottomLeft();
		}
		else if(cornerType == SelectionRect::BottomRight) {
			p1 = selectionRect->topLeft();
			p2 = selectionRect->bottomRight();
		}
		cornerType = SelectionRect::NoCorner;
	}
	else if(e->buttons() == Qt::RightButton && selectionRect->contains(e->pos())) {
		QMenu m;
		m.addAction(tr("Cut"), this, SLOT(cut()));
		m.addAction(tr("Copy"), this, SLOT(copy()));
		m.exec(e->globalPos());
	}
	e->accept();
}

void PixmapWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
	if(selectionRect->contains(e->pos())
		&& e->buttons() == Qt::LeftButton)
		cut();

	e->accept();
}

void PixmapWidget::mouseReleaseEvent(QMouseEvent *e)
{

	if(e->buttons() != !Qt::LeftButton ) {
		e->accept();
		return;
	}

	if(type_ == ToolBar::ButtonPen) {
		setAttribute(Qt::WA_OpaquePaintEvent, false);
		paintToPixmap();
	}
	else if(type_ == ToolBar::ButtonEllipse || type_ == ToolBar::ButtonRect) {
		paintToPixmap();
	}
	else if(type_ == ToolBar::ButtonText) {
		selectionRect->setCoords(qMin(p1.x(), p2.x()), qMin(p1.y(), p2.y()),
					 qMax(p1.x(), p2.x()), qMax(p1.y(), p2.y()));
		GetTextDlg gtd(this);
		connect(&gtd, SIGNAL(text(QString)), SLOT(paintToPixmap(QString)));
		connect(&gtd, SIGNAL(selectFont()), SLOT(selectFont()));
		gtd.exec();
	}
	else if(type_ == ToolBar::ButtonSelect && p1 != e->pos() && p1.x() != -1) {
		selectionRect->setCoords(qMin(p1.x(), p2.x()), qMin(p1.y(), p2.y()),
					 qMax(p1.x(), p2.x()), qMax(p1.y(), p2.y()));
	}

	p1 = QPoint(-1, -1);
	p2 = QPoint(-1, -1);
	e->accept();
}

void PixmapWidget::mouseMoveEvent(QMouseEvent *e)
{
	if(e->buttons() == Qt::LeftButton) {
		if(type_ == ToolBar::ButtonPen) {
			p1 = p2;
			p2 = e->pos();
			if(e->modifiers() == Qt::ShiftModifier) {
				if(smoothLineType_ == None) {
					if(qAbs(p1.x() - p2.x()) > qAbs(p1.y() - p2.y())) {
						smoothLineType_ = Horizontal;
					}
					else {
						smoothLineType_ = Vertical;
					}
				}
				switch(smoothLineType_) {
				case Horizontal:
					p2.setY(p1.y());
					break;
				case Vertical:
					p2.setX(p1.x());
					break;
				case None:
					break;
				}
			}
			paintToPixmap();
		}
		else if(type_ == ToolBar::ButtonSelect || type_ == ToolBar::ButtonText
			|| type_ == ToolBar::ButtonEllipse || type_ == ToolBar::ButtonRect) {
			if(cornerType == SelectionRect::NoCorner) {
				if(e->pos().x() >= 0 && e->pos().y() >= 0) {
					p2 = e->pos();
				}
			}
			update();
		}
	}
	else if(e->buttons() == Qt::NoButton) {
		static bool cursorModified = false;
		cornerType = selectionRect->cornerUnderMouse(e->pos());
		if(cornerType == SelectionRect::TopLeft || cornerType == SelectionRect::BottomRight) {
			setCursor(QCursor(Qt::SizeFDiagCursor));
			cursorModified = true;
		}
		else if(cornerType == SelectionRect::TopRight || cornerType == SelectionRect::BottomLeft) {
			setCursor(QCursor(Qt::SizeBDiagCursor));
			cursorModified = true;
		}
		else if(cursorModified) {
			setCursor(currentCursor);
			cursorModified = false;
		}
	}

	e->accept();
}

void PixmapWidget::paintToPixmap(QString text)
{
	QPainter painter;
	painter.begin(&mainPixmap);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setPen(pen);

	if(type_ == ToolBar::ButtonPen) {
		if(p1.x() != -1 && p2.x() != -1) {
			if(p1 != p2)
				painter.drawLine(p1, p2);
			else
				painter.drawPoint(p1);
		}
	}
	else if(type_ == ToolBar::ButtonText && !text.isEmpty() && selectionRect->x() != -1) {
		saveUndoPixmap();
		painter.setFont(font_);
		painter.drawText((QRect)*selectionRect, text);
		selectionRect->clear();
	}
	else if(p2.x() != -1) {
		saveUndoPixmap();
		int w = (p2.x() - p1.x());
		int h = (p2.y() - p1.y());
		if(type_ == ToolBar::ButtonEllipse) {
			painter.drawEllipse(p1.x(), p1.y(), w, h);
		}
		else if(type_ == ToolBar::ButtonRect) {
			painter.drawRect(p1.x(), p1.y(), w, h);
		}
	}

	painter.end();
	update();
}

void PixmapWidget::selectColor()
{
	QColorDialog cd;
	cd.setCurrentColor(color_);
	if(cd.exec() == QDialog::Accepted) {
		color_ = cd.currentColor();
		pen.setColor(color_);
		bar_->setColorForColorButton(color_);
		settingsChanged(constColor, QVariant(color_.name()));
	}
}

void PixmapWidget::selectFont()
{
	bool *ok = 0;
	font_ = QFontDialog::getFont(ok, font_, this);
	settingsChanged(constFont, QVariant(font_.toString()));
}

void PixmapWidget::undo()
{
	if(!undoList_.isEmpty()) {
		const QPixmap pix = undoList_.takeLast();
		bool doAdjust = ( pix.size() !=  mainPixmap.size() );
		setPixmap(pix);
		if(doAdjust)
			emit adjusted();
	}
	bool hasUndo = !undoList_.isEmpty();
	if(!hasUndo) {
		bar_->enableButton(hasUndo, ToolBar::ButtonUndo);
		emit modified(hasUndo);
	}
}

void PixmapWidget::saveUndoPixmap()
{
	undoList_.append(mainPixmap);
	bar_->enableButton(true, ToolBar::ButtonUndo);
	emit modified(true);
}

void PixmapWidget::checkedButtonChanged(ToolBar::ButtonType type)
{
	switch(type) {
	case(ToolBar::ButtonPen):
		currentCursor = QCursor(QPixmap(":/icons/icons/draw.png"), 2,15);
		break;
	case(ToolBar::ButtonSelect):
	case(ToolBar::ButtonEllipse):
	case(ToolBar::ButtonRect):
	case(ToolBar::ButtonText):
		currentCursor = QCursor(Qt::CrossCursor);
		break;
	default:
		currentCursor = QCursor(Qt::ArrowCursor);
		break;
	}

	setCursor(currentCursor);
	selectionRect->clear();
	type_ = type;
	cornerType = SelectionRect::NoCorner;

	update();
}

#include "pixmapwidget.moc"
