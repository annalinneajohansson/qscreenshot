/*
 * translator.cpp
 * Copyright (C) 2011-2013  Khryukin Evgeny
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

#include <QApplication>
#include <QLocale>
#include <QDir>
#include <QStringList>
#include <QLibraryInfo>

#include "translator.h"
#include "options.h"
#include "defines.h"


#ifndef QSCREENSHOT_DATADIR
#define QSCREENSHOT_DATADIR "/usr/local/share/qscreenshot"
#endif

static const QString PREFIX = "qscreenshot_";

Translator* Translator::instance_ = 0;

Translator* Translator::instance()
{
	if(!instance_) {
		instance_ = new Translator();
	}

	return instance_;
}

Translator::Translator()
	: QTranslator(qApp)
	, qtTrans_(new QTranslator())
{
	QVariant vCur = Options::instance()->getOption(constCurLang);
	if(vCur == QVariant::Invalid) {
		vCur = QLocale::system().name().split("_").first();
	}
	retranslate(vCur.toString());
}

Translator::~Translator()
{
	delete qtTrans_;
	qtTrans_ = 0;
}

void Translator::reset()
{
	delete instance_;
	instance_ = 0;
}

void Translator::retranslate(const QString& fileName)
{
	bool foundFile = false;
	foreach(const QString& dir, transDirs()) {
		if(load(PREFIX+fileName, dir)) {
			qApp->installTranslator(this);
			foundFile = true;

			QStringList dirs;
			dirs << QLibraryInfo::location(QLibraryInfo::TranslationsPath) << dir;
			foreach(const QString& d, dirs) {
				if(qtTrans_->load("qt_"+fileName, d)) {
					qApp->installTranslator(qtTrans_);
					break;
				}
			}

			break;
		}
	}
	if(!foundFile) {
		qApp->removeTranslator(this);
		qApp->removeTranslator(qtTrans_);
	}

	setCurrentTranslation(fileName);
}

QStringList Translator::availableTranslations()
{
	QStringList translations("en"); // add default translation
	foreach(const QString& dir, transDirs()) {
		foreach(QString file, QDir(dir).entryList(QDir::Files)) {
			if(file.startsWith(PREFIX, Qt::CaseInsensitive)) {
				file.chop(3);
				file.remove(PREFIX);
				translations.append(file);
			}
		}
	}

	return translations;
}

QString Translator::currentTranslation() const
{
	return Options::instance()->getOption(constCurLang, "en").toString();
}

void Translator::setCurrentTranslation(const QString &tr)
{
	Options::instance()->setOption(constCurLang, tr);
}

QStringList Translator::transDirs()
{
	static QStringList list;
	if(list.isEmpty()) {
#ifdef Q_OS_WIN
		list.append(qApp->applicationDirPath()+"/translations");
#elif defined (HAVE_X11)
		list.append(QString(QSCREENSHOT_DATADIR) + "/translations");
#elif defined (Q_OS_MAC)
		QDir appDir = qApp->applicationDirPath();
		appDir.cdUp();
		appDir.cd("Translations");
		list.append(appDir.absolutePath());
#endif
	}
	return list;
}
