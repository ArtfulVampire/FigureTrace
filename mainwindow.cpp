#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QEvent>
#include <QMouseEvent>
#include <QFileDialog>
#include <QTime>

#include <fstream>

std::ostream & operator << (std::ostream & os, const QPoint & in)
{
	os << in.x() << "\t" << in.y();
	return os;
}

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	QPixmap peec(ui->picLabel->size());
	peec.fill("black");
	ui->picLabel->setPixmap(peec);
	ui->picLabel->setMouseTracking(true);
	ui->picLabel->installEventFilter(this);

	comPort = new QSerialPort(this);
	comPort->setPortName("COM1");
	comPort->open(QIODevice::WriteOnly);
	comPortDataStream.setDevice(comPort);


	QObject::connect(ui->openFilePushButton, &QPushButton::clicked,
					 [this]()
	{
		clearAll();
		QString fName = QFileDialog::getOpenFileName(this,
													 tr("Open figure file"),
													 "/media/Files/Data/Tracking");
		if(!fName.isEmpty())
		{
			ui->picLabel->setPixmap(drawFigure(currFigure = loadFigure(fName)));
		}
	});

	QObject::connect(ui->saveFilePushButton, &QPushButton::clicked,
					 [this]()
	{
		QString fName = QFileDialog::getSaveFileName(this,
													 tr("File to save"),
													 "/media/Files/Data/Tracking");
		if(!fName.isEmpty())
		{
			saveFigure(fName, currTracking);
		}
	});

	QObject::connect(ui->clearPushButton, &QPushButton::clicked,
					 this, &MainWindow::clearAll);

//	std::cout << trackingQuality(loadFigure("/media/Files/Data/Tracking/2.txt"),
//								 loadFigure("/media/Files/Data/Tracking/2_.txt"))
//			  <<std::endl;
//	exit(0);

}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::clearAll()
{
	QPixmap peec(ui->picLabel->size());
	peec.fill("black");
	ui->picLabel->setPixmap(peec);

	pic.fill("black");
	if(pnt.device()) { pnt.end(); }
	currTracking.clear();
	currFigure.clear();
	tracking = false;
}

std::vector<QPoint> MainWindow::loadFigure(const QString & filePath)
{
	std::vector<QPoint> res{};
	std::ifstream inStr(filePath.toStdString());
	for(int x, y; inStr >> x >> y;)
	{
		res.push_back(QPoint(x, y));
	}
	inStr.close();
	return res;
}

QPixmap MainWindow::drawFigure(const std::vector<QPoint> & in)
{
	QPixmap res(ui->picLabel->size());
	res.fill("black");
	QPainter paint;
	paint.begin(&res);
	paint.setPen(QPen(QBrush("white"), 2));

	for(int i = 0; i < in.size() - 1; ++i)
	{
		paint.drawLine(in[i].x(), in[i].y(), in[i + 1].x(), in[i + 1].y());
	}
	paint.end();
	return res;
}

void MainWindow::saveFigure(const QString & filePath, const std::vector<QPoint> & in)
{
	std::ofstream outStr(filePath.toStdString());
	for(const QPoint & p : in)
	{
		outStr << p.x() << "\t" << p.y() << "\n";
	}
	outStr.close();
}

double MainWindow::trackingQuality(const std::vector<QPoint> & fig,
								   const std::vector<QPoint> & track)
{
	std::vector<double> res{};
	for(const auto & figPoint : fig)
	{
		double dist = 500.;
		for(const auto & trackPoint : track)
		{
			QPoint a = figPoint - trackPoint;
			double d = std::sqrt(QPoint::dotProduct(a, a));
			dist = std::min(dist, d);

//			std::cout << figPoint << std::endl;
//			std::cout << trackPoint << std::endl;
//			std::cout << a << std::endl;
//			std::cout << d << std::endl;
//			std::cout << std::endl;

			if(dist < 5 && d > 10) break;
		}
//		return dist;
		res.push_back(dist);
	}
	/// average distance of best fit
	const double ret = std::accumulate(std::begin(res), std::end(res), 0.) / res.size();

	/// mapped [lowest-highest] -> [100-0]
	const double lowest = 1;
	const double highest = 6;
	return (highest - ret) / (highest - lowest) * 100;
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
	if(obj == ui->picLabel)
	{
		static QPoint prevPos{};
		static QPoint initPos{};

		QMouseEvent * ev = static_cast<QMouseEvent*>(event);
		switch(event->type())
		{
		case QEvent::MouseButtonRelease:
		{
			if(!tracking)
			{
				initPos = ev->pos();
				prevPos = ev->pos();
				currTracking.clear();
				pic = *ui->picLabel->pixmap();
				pnt.begin(&pic);
				pnt.setPen(QPen(QBrush("gray"), 2));
				sta = std::chrono::system_clock::now();
				comPortDataStream << startCode;
			}
			else
			{
				pnt.drawLine(ev->pos(), initPos);
				ui->picLabel->setPixmap(pic);
				if(ui->trackRadioButton->isChecked())
				{
					comPortDataStream << finishCode;
					using chr = std::chrono;
					fin = chr::system_clock::now();
					std::cout << "quality = "
							  << trackingQuality(currFigure, currTracking)
							  << std::endl;
					std::cout << "time = "
							  << chr::duration_cast<chr::milliseconds>(fin - sta).count() / 1000.
//							  << " sec"
							  << std::endl;
				}
				else if(ui->drawRadioButton->isChecked())
				{
					/* do nothing? */
				}
			}
			tracking = !tracking;
			return true;
			break;
		}
		case QEvent::MouseMove:
		{
			if(tracking)
			{
//				std::cout << ev->pos().x() << "\t" << ev->pos().y() << std::endl;
				currTracking.push_back(ev->pos());
				pnt.drawLine(prevPos, ev->pos());
				ui->picLabel->setPixmap(pic);
				prevPos = ev->pos();
			}
			return true;
			break;
		}
		default:
		{
			return QWidget::eventFilter(obj, event);
		}
		}
	}
	return QWidget::eventFilter(obj, event);
}
