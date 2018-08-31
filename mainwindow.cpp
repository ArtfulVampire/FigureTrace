#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "lib.h"

#include <memory>
#include <cmath>

#include <QEvent>
#include <QMouseEvent>
#include <QFileDialog>
#include <QTime>
#include <QMessageBox>
#include <QShortcut>

namespace chr = std::chrono;

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
//	std::cout << this->size().width() << std::endl;

	ui->picLabel->setMouseTracking(true);

	this->installEventFilter(this);
	ui->picLabel->installEventFilter(this);

	comPort = new QSerialPort(this);
	comPort->setPortName("COM1");
	comPort->open(QIODevice::WriteOnly);
	comPortDataStream.setDevice(comPort);

	ui->dontEnlargeCheckBox->setChecked(false);


	clearScreen();
	reassignPic();

	QObject::connect(this, &MainWindow::completed,
					 [this]() { ++fileIndex; openNextFile(); });

	QObject::connect(ui->makeTxtPushButton,
					 &QPushButton::clicked,
					 [this]()
	{
		QString picPath = QFileDialog::getOpenFileName(this,
													   tr("Choose a picture to make txt"),
													   defPath,
													   "*.png, *.jpg");
		if(!picPath.isEmpty())
		{
			QMessageBox::information(this,
									 tr("Info"),
									 tr(R"(The process can take about a minute.)"));
			auto figure = readFromPicture(picPath,
										  ui->lineThicknessSpinBox->value(),
										  ui->numStepsSpinBox->value(),
										  ui->stepSizeSpinBox->value());
			figure = approximateCurve(figure);
			QString outPath = QFileDialog::getSaveFileName(this,
														   tr("Choose output fileName"),
														   defPath);
			while(outPath.isEmpty())
			{
				outPath = QFileDialog::getSaveFileName(this,
											 tr("Choose output fileName"),
											 defPath);
				QMessageBox::critical(this,
									  tr("Error"),
									  tr("Output fileName can't be empty"),
									  QMessageBox::Ok);
			}
			if(!outPath.endsWith(".txt")) { outPath += ".txt"; }
			saveFigure(outPath, figure);
		}
	});
	QObject::connect(ui->justOpenPushButton, &QPushButton::clicked,
					 [this]() { justOpenSlot(); });




#if 0
	const QString picName = "hedge";
//	thresholding(defPath + "/" + picName + ".jpg").save(defPath + "/" + picName + "_1.jpg", 0, 100);
//	makeThinnerLine(defPath + "/" + picName + "_1.jpg", false, 4).save(defPath + "/" + picName + "_2.jpg", 0, 100);

//	auto init = loadFigure(defPath + "/hippocampus_adj.txt");
//	drawFigure(init,
//			   ui->picLabel->size(),
//			   2).save(defPath + "/" + picName + "_init.jpg", 0, 100);
//	auto trac = loadFigure(defPath + "/tr2.txt");
//	drawFigure(trac,
//			   ui->picLabel->size(),
//			   2).save(defPath + "/" + picName + "_trac.jpg", 0, 100);
//	std::cout << trackingQuality(trac, init) << std::endl; exit(0);


	auto t1 = chr::high_resolution_clock::now();
	currFigure = readFromPicture(defPath + "/" + picName + ".jpg", 14, 4, 2);
	auto t2 = chr::high_resolution_clock::now();
	std::cout << chr::duration_cast<chr::milliseconds>(t2-t1).count() << std::endl;
//	currFigure = smoothCurve(currFigure);
//	saveFigure(defPath + "/" + picName + ".txt", currFigure);

//	drawFigure(loadFigure(defPath + "/" + picName + ".txt"),
//			   QPixmap(defPath + "/" + picName + ".jpg").size(),
//			   2).save(defPath + "/" + picName + "_new.jpg", 0, 100);
	exit(0);
#endif

#if 0
	currFigure = loadFigure(defPath + "/1.txt");
	adjustCurve(currFigure, QSize(1200, 800), ui->dontEnlargeCheckBox->isChecked());
	drawFigure(currFigure, QSize(1200, 800), 2).save(defPath + "/1.jpg");

	exit(0);
#endif

}

MainWindow::~MainWindow()
{
	delete ui;
}


void MainWindow::retrySlot()
{
	if(!currFigure.empty())
	{
		ui->picLabel->setPixmap(drawFigure(currFigure, ui->picLabel->size()));
	}
//	pic.fill("black");
//	if(pnt.device()) { pnt.end(); }
	currTracking.clear();
	tracking = false;
	timerCount = 0.;
}

void MainWindow::saveSlot()
{
	QString fName = QFileDialog::getSaveFileName(this,
												 tr("File to save"),
												 defPath);
	if(!fName.isEmpty())
	{
		saveFigure(fName, currTracking);
	}
}

void MainWindow::openNextFile()
{
	clearSlot();


	if(fileIndex == fileNames.end())
	{
		outStr.close();
		QMessageBox::information(this,
								 tr("Congratulations!"),
								 tr("You have finished all the tasks"),
								 QMessageBox::Ok);
		return;
	}
	else
	{
		QMessageBox::information(this,
								 tr("Info"),
								 tr("Press OK for the next task"),
								 QMessageBox::Ok);
	}

	QString fName = *fileIndex;
	{
		if(fName.endsWith(".txt", Qt::CaseInsensitive))
		{
			currFigure = loadFigure(fName);
			currFigure = adjustCurve(currFigure, ui->picLabel->size(), ui->dontEnlargeCheckBox->isChecked());
			ui->picLabel->setPixmap(drawFigure(currFigure, ui->picLabel->size()));
			reassignPic();
		}
		else // if(fName.endsWith(QRegExp(R"(bmp|jpg|jpeg|tiff|png)")))
		{
			QMessageBox::warning(this,
								 tr("Warning"),
								 tr("Prepare txt file offline"),
								 QMessageBox::Ok);
		}
	}
}



void MainWindow::justOpenSlot()
{
	clearSlot();
	QString fName = QFileDialog::getOpenFileName(this,
												 tr("Open figure file"),
												 defPath,
												 "*.txt");
	currFigure = loadFigure(fName);
	currFigure = adjustCurve(currFigure, ui->picLabel->size(), ui->dontEnlargeCheckBox->isChecked());
	ui->picLabel->setPixmap(drawFigure(currFigure, ui->picLabel->size()));
	reassignPic();
}

void MainWindow::openSlot()
{
	/// remake to list
	fileNames = QFileDialog::getOpenFileNames(this,
											  tr("Open figure file"),
											  defPath,
											  "*.txt");
	if(fileNames.isEmpty()) { return; }
	fileIndex = std::begin(fileNames);


	static int outFileNum{0};
	auto outFilePath = [](int i) -> QString
	{
		return "./output_" + QString::number(i) + ".txt";
	};
	for(; QFile(outFilePath(outFileNum)).exists(); ++outFileNum);

	outStr.open(outFilePath(outFileNum).toStdString());
	outStr
			<< "fileName  " << "\t"
			<< "avDistance" << "\t"
			<< "quality   " << "\t"
			<< "time      " << "\t"
			<< "numPauses " << "\t"
			<< std::endl;

	openNextFile();
}

void MainWindow::reassignPic()
{
	if(pnt.device()) pnt.end();
	pic = *(ui->picLabel->pixmap());
	pnt.begin(&pic);
	pnt.setPen(QPen(QBrush("gray"), 2));
}

void MainWindow::clearScreen(const QColor & color)
{
	QPixmap peec(ui->picLabel->size());
	peec.fill(color);
	ui->picLabel->setPixmap(peec);
}

void MainWindow::clearSlot()
{
	clearScreen();
	pic.fill("black");

	currTracking.clear();
	currFigure.clear();
	tracking = false;
	timerCount = 0.;
	numPauses = 0;
}

void MainWindow::addDeveloperStuff(bool add)
{
	static bool is{false};
	const int addWidth = 200;

	if(add && !is)
	{
		this->resize(size().width() + addWidth, size().height());
		is = true;
	}
	else if(!add && is)
	{
		this->resize(size().width() - addWidth, size().height());
		is = false;
	}

}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
	if(event->type() == QEvent::KeyPress)
	{
		QKeyEvent * ev = static_cast<QKeyEvent*>(event);
		switch(ev->key())
		{
		case Qt::Key_O : { openSlot(); return true; }
		case Qt::Key_S : { saveSlot(); return true; }
		case Qt::Key_C : { clearSlot(); return true; }
		case Qt::Key_R : { retrySlot(); return true; }
		case Qt::Key_D : { mode = Mode::draw; return true; }
		case Qt::Key_T : { mode = Mode::track; return true; }
		case Qt::Key_Escape : { this->close(); return true; }
		case Qt::Key_X :
		{
			if(ev->modifiers().testFlag(Qt::ControlModifier))
			{
				addDeveloperStuff(true);
			}
			else if(ev->modifiers().testFlag(Qt::ShiftModifier))
			{
				addDeveloperStuff(false);
			}
			return true;
		}

		default: { /* do nothing */ }
		}
	}
	if(obj == ui->picLabel)
	{
		static QPoint prevPos{};
		static QPoint initPos{};

		QMouseEvent * ev = static_cast<QMouseEvent*>(event);
		switch(event->type())
		{
		case QEvent::MouseButtonRelease:
		{
			if(ev->button() == Qt::LeftButton)
			{
				if(!tracking)
				{
					initPos = ev->pos();
					prevPos = ev->pos();
					sta = chr::system_clock::now();
					comPortDataStream << startCode;
				}
				else
				{
					ui->picLabel->setPixmap(pic);
					fin = chr::system_clock::now();
					timerCount += chr::duration_cast<chr::milliseconds>(fin - sta).count();
					++numPauses;
				}
				tracking = !tracking;
			}
			else /// if(ev->button() == Qt::RightButton)
			{
				if(mode == Mode::track)
				{
					tracking = false;
					comPortDataStream << finishCode;
					double qual1 = trackingQuality(currFigure, currTracking);
					double qual2 = trackingQuality(currTracking, currFigure);
					outStr
							<< (*fileIndex).mid((*fileIndex).lastIndexOf('/') + 1) << "\t\t"
							<< myRound(std::min(qual1, qual2), 1) << "\t\t"
							<< myRound(quality(std::min(qual1, qual2)), 0) << "\t\t"
							<< myRound(timerCount / 1000., 1) << "\t\t"
							<< numPauses << "\t\t"
							<< std::endl;

					emit completed();
				}
				else if(mode == Mode::draw)
				{
					//						currTracking = smoothCurve(currTracking, 3);
					ui->picLabel->setPixmap(drawFigure(currTracking, ui->picLabel->size()));
				}
			}
			return true;
			break;
		}
		case QEvent::MouseMove:
		{
			if(tracking)
			{
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

