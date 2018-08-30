#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPixmap>
#include <QPainter>
#include <QSerialPort>

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

protected:
	bool eventFilter(QObject * obj, QEvent * event);

public slots:
	void clearSlot();
	void openSlot();
	void openNextFile();
	void saveSlot();
	void retrySlot();

signals:
	void completed();

private:
	void clearScreen(const QColor & color = Qt::black);
	void reassignPic();
	void addDeveloperStuff(bool add);

private:
	Ui::MainWindow *ui;

	bool tracking{false};		/// is currently tracked
	enum class Mode {draw, track};
	Mode mode{Mode::track};

	QPixmap pic;
	QPainter pnt;

	QStringList fileNames{};
	QStringList::iterator fileIndex{};
	std::vector<QPoint> currFigure{};
	std::vector<QPoint> currTracking{};

	std::chrono::system_clock::time_point sta{};
	std::chrono::system_clock::time_point fin{};
	int timerCount{0};

	QSerialPort * comPort = nullptr;
	QDataStream comPortDataStream{};
	static const qint8 startCode = 241;
	static const qint8 finishCode = 247;
};

#endif // MAINWINDOW_H
