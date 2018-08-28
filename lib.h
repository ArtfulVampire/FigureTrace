#ifndef LIB_H
#define LIB_H

#include <iostream>
#include <vector>

#include <QPixmap>
#include <QPoint>
#include <QColor>

const QSize picSiz(650, 650);
const QString defPath = "/media/Files/Data/Tracking";

enum class direction : int {NW = 0, NN, NE, EE, SE, SS, SW, WW};
direction operator++(direction & in, int);
direction operator+(const direction & in, int a);
direction operator-(const direction & in, int a);
direction operator--(direction & in, int);
direction opposite(const direction & in);
QPoint getDs(const direction & in);
QPoint operator+(const direction & a, const direction & b);
QPoint operator+(const direction & a, const QPoint & b);
QPoint operator+(const QPoint & a, const direction & b);

std::ostream & operator<< (std::ostream & os, const QPoint & in);

template <typename pointType>
double trackingQuality(const std::vector<pointType> & fig,
					   const std::vector<pointType> & track);

template <typename pointType>
double trackingQualityInner(const std::vector<pointType> & fig,
							const std::vector<pointType> & alignedTrack);

template <typename pointType>
std::vector<pointType>
alignTracking(const std::vector<pointType> & fig,
			  const std::vector<pointType> & track);

template <typename pointType>
std::vector<pointType>
smoothCurve(const std::vector<pointType> & curve);

std::vector<QPoint> loadFigure(const QString & filePath);

template <typename pointType>
std::vector<pointType> approximateCurve(const std::vector<pointType> & in);

QPixmap drawFigure(const std::vector<QPoint> & in);


QImage makeThinnerLine(const QString & picPath, bool isLineWhite, int num);
QImage thresholding(const QString & picPath);


bool areCloseEnough(const QColor & in1, const QColor & in2, int thr = 15);
std::vector<QPoint> readFromPicture(const QString & picPath);

template <typename pointType>
void saveFigure(const QString & filePath, const std::vector<pointType> & in);

#endif // LIB_H
