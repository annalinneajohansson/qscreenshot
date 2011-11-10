/*
 * options.cpp
 * Copyright (C) 2011  Khryukin Evgeny
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


#include "options.h"
#include <QSettings>
#include <QApplication>


Options* Options::instance_ = 0;

Options* Options::instance()
{
	if(!instance_) {
		instance_ = new Options();
	}

	return instance_;
}

Options::Options()
	: QObject(0)
	, set_(0)
{
	set_ = new QSettings(qApp->organizationName(), qApp->applicationName(), this);
}

Options::~Options()
{
	set_->sync();
}

void Options::reset()
{
	instance_->deleteLater();
	instance_ = 0;
}

QVariant Options::getOption(const QString& name, const QVariant& defValue)
{
	QVariant val = set_->value(name, defValue);
	return val;
}

void Options::setOption(const QString& name, const QVariant& value)
{
	set_->setValue(name, value);
}
