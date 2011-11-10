/*
 * screenshot.h
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

#ifndef SCREENSHOT_H
#define SCREENSHOT_H

#include <QMainWindow>
#include <QPointer>

struct Proxy;
class Server;
class GrabAreaWidget;
class QNetworkAccessManager;
class QNetworkReply;
class QLabel;

namespace Ui {
	class Screenshot;
}

class Screenshot : public QMainWindow
{
    Q_OBJECT

public:
	Screenshot();
	~Screenshot();

protected:
	void closeEvent(QCloseEvent *);

public slots:
	void shootScreen();
	void openImage();
	void newScreenshot();
	void action();

	void doOptions();
	void doProxySettings();

private slots:
	void saveScreenshot();
	void uploadScreenshot();
	void printScreenshot();
	void cancelUpload();
	void dataTransferProgress( qint64 done, qint64 total );
	void ftpReplyFinished();
	void httpReplyFinished(QNetworkReply*);
	void captureDesktop(int);
	void captureWindow(int);
	void captureArea(int);	
	void shootWindow();
	void shootArea();
	void screenshotCanceled();
	void pixmapAdjusted();
	void fixSizes();
	void setModified(bool m);
	void aboutQt();
	void doHomePage();
	void doHistory();
	void doAbout();
	void doCheckUpdates();
	void settingsChanged(const QString& option, const QVariant& value);
	void copyUrl();

private:
	void updateScreenshotLabel();
	void refreshWindow();
	void uploadFtp();
	void uploadHttp();
	void bringToFront();
	void updateWidgets(bool vis);
	void connectMenu();
	void setServersList(const QStringList& servers);
	void setImagePath(const QString& path);
	void refreshSettings();
	void saveGeometry();
	void setProxy();
	void shoot(WId id);
	void newRequest(const QNetworkReply *const old, const QString& link);
	void setupStatusBar();
	void updateStatusBar();

	bool modified;
	QPixmap originalPixmap;
	QString format;
	QString fileNameFormat;
	QString lastFolder;
	QList<Server*> servers;
	QPointer<QNetworkAccessManager> manager;
	QByteArray ba;
	Proxy *proxy_;
	QStringList history_;
	GrabAreaWidget* grabAreaWidget_;
	QLabel *sbLbSize;

	Ui::Screenshot *ui_;
};


#endif
