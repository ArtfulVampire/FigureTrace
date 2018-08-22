#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPixmap>
#include <QPainter>
#include <QSerialPort>

#include <iostream>
#include <cmath>
#include <chrono>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget * parent = 0);
	~MainWindow();

	std::vector<QPoint> loadFigure(const QString & filePath);
	QPixmap drawFigure(const std::vector<QPoint> & in);
	void saveFigure(const QString & filePath, const std::vector<QPoint> & in);

public slots:

	void clearAll();

protected:
	bool eventFilter(QObject * obj, QEvent * event);

private:
	Ui::MainWindow *ui;
	bool tracking;		/// is currently tracked
	QPixmap pic;
	QPainter pnt;
	std::chrono::system_clock::time_point sta{};
	std::chrono::system_clock::time_point fin{};

	QSerialPort * comPort = nullptr;
	QDataStream comPortDataStream{};

	std::vector<QPoint> currFigure{};
	std::vector<QPoint> currTracking{};

	static const qint8 startCode = 241;
	static const qint8 finishCode = 247;

	double trackingQuality(const std::vector<QPoint> & fig,
						   const std::vector<QPoint> & track);
};

#endif // MAINWINDOW_H
