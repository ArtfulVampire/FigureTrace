#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "lib.h"

#include <QEvent>
#include <QMouseEvent>
#include <QFileDialog>
#include <QTime>

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	ui->picLabel->resize(picSiz);
	QPixmap peec(picSiz);
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

	QObject::connect(ui->tryAgainPushButton, &QPushButton::clicked,
					 [this]()
	{
		if(!currFigure.empty())
		{
		   ui->picLabel->setPixmap(drawFigure(currFigure));
		}
		pic.fill("black");
		if(pnt.device()) { pnt.end(); }
		currTracking.clear();
		tracking = false;
	});

#if 0
	std::cout << trackingQualityInner(loadFigure("/media/Files/Data/Tracking/5.txt"),
									  loadFigure("/media/Files/Data/Tracking/tr1.txt"))
			  << std::endl;
	std::cout << trackingQualityInner(loadFigure("/media/Files/Data/Tracking/5.txt"),
									  loadFigure("/media/Files/Data/Tracking/tr2.txt"))
			  << std::endl;
	exit(0);
#endif
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::clearAll()
{
	QPixmap peec(picSiz);
	peec.fill("black");
	ui->picLabel->setPixmap(peec);

	pic.fill("black");
	if(pnt.device()) { pnt.end(); }
	currTracking.clear();
	currFigure.clear();
	tracking = false;
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
				pic = *(ui->picLabel->pixmap());
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
					namespace chr = std::chrono;
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
				return true;
			}
			return true;
		}
		default:
		{
			return QWidget::eventFilter(obj, event);
		}
		}
	}
	return QWidget::eventFilter(obj, event);
}
