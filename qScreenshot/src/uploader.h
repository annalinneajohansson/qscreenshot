/*
 * Copyright (C) 2009-2013  Khryukin Evgeny
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

#ifndef UPLOADER_H
#define UPLOADER_H

#include <QPixmap>

class Server;
class QNetworkAccessManager;
class QNetworkReply;

class Uploader : public QObject
{
	Q_OBJECT
public:
	explicit Uploader(Server* serv, QObject *parent = 0);
	~Uploader();

	void uploadFtp(const QPixmap& pixmap);
	void uploadHttp(const QPixmap& pixmap);

signals:
	void requestFinished();
	void newUrl(const QString&);
	void addToHistory(const QString&);
	void uploadProgress(qint64 , qint64);
	
private slots:
	void ftpReplyFinished();
	void httpReplyFinished(QNetworkReply*);

private:
	void newRequest(const QNetworkReply *const old, const QString& link);

private:
	QNetworkAccessManager* manager_;
	QByteArray ba_;
	Server* server_;
};

#endif // UPLOADER_H
