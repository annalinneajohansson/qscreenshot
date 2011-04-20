/*
 * server.h
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

#ifndef SERVER_H
#define SERVER_H

#include <QListWidgetItem>

class Server : public QListWidgetItem, public QObject
{
public:
	Server(QListWidget *parent = 0);
	QString displayName() const { return displayName_; };
	QString url() const { return url_; };
	QString userName() const { return userName_; };
	QString password() const { return password_; };
	QString servPostdata() const { return servPostdata_; };
	QString servFileinput() const { return servFileinput_; };
	QString servRegexp() const { return servRegexp_; };
	//QString servFilefilter() { return servFilefilter_; };
	void setServer(const QString& url, const QString& user = "", const QString& pass = "");
	void setServerData(const QString& post = "", const QString& fInput = "", const QString& reg = ""/*, QString fFilter = ""*/);
	void setDisplayName(QString n);
	void setUseProxy(bool use) { useProxy_ = use; };
	bool useProxy() const { return useProxy_; };

	QString settingsToString() const ;
	void setFromString(const QString& settings);

	static QString splitString();

private:
	QString displayName_;
	QString url_, userName_, password_;
	QString servPostdata_, servFileinput_, servRegexp_/*, servFilefilter_*/;
	bool useProxy_;

	void processOltSettingsString(QStringList l);
};

#endif // SERVER_H
