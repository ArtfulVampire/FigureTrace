#ifndef LIB_H
#define LIB_H

#include <iostream>
#include <vector>

#include <QPixmap>
#include <QPoint>
#include <QColor>
#include <QDir>

//const QString defPath = "/media/Files/Data/Tracking";
const QString defPath = ".";

enum class direction : int {NW = 0, NN, NE, EE, SE, SS, SW, WW};
direction operator++(direction & in, int);
direction operator+(const direction & in, int a);
direction operator-(const direction & in, int a);
direction operator--(direction & in, int);
QPoint operator*(const direction & in, int a);
QPoint operator*(int a, const direction & in);
direction opposite(const direction & in);
QPoint getDs(const direction & in);
direction dirFromPoint(const QPoint & pt);
direction dirFromInt(int in);
QPoint operator+(const direction & a, const direction & b);
QPoint operator+(const direction & a, const QPoint & b);
QPoint operator+(const QPoint & a, const direction & b);

std::ostream & operator<< (std::ostream & os, const QPoint & in);
std::ostream & operator<< (std::ostream & os, const QPointF & in);
std::ostream & operator<< (std::ostream & os, const direction & in);
std::ostream & operator<< (std::ostream & os, const QString & in);

double myRound(double in, int decims = 2);

double quality(double in, double low = 2, double hig = 20);

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

template <typename pointType>
std::vector<pointType>
adjustCurve(const std::vector<pointType> & curve,
			const QSize & size,
			bool dontEnlarge);

std::vector<QPoint> loadFigure(const QString & filePath);

template <typename pointType>
std::vector<pointType> approximateCurve(const std::vector<pointType> & in);

QPixmap drawFigure(const std::vector<QPoint> & in,
				   const QSize & size,
				   int lineWidth = 2);


QImage makeThinnerLine(const QString & picPath, bool isLineWhite, int num);
QImage thresholding(const QString & picPath);

int numPointsMask(const QImage & pic, const QPoint & pt,
				  const QColor & curveColor, int lineThickness);
bool areColorsSimilar(const QColor & in1, const QColor & in2, int thr = 15);

std::vector<QPoint> readFromPicture(const QString & picPath,
									int lineThickness = 1,
									int numSteps = 5,
									int stepSize = 1);

std::vector<QPoint> readFromPictureSimple(const QString & picPath); /// lineWidth < 4

std::list<std::list<int>> generateDirectionVariants(int numSteps = 4);

template <typename pointType>
void saveFigure(const QString & filePath, const std::vector<pointType> & in);

template <typename Cont, typename Val>
bool contains(const Cont & cont, Val val);

#endif // LIB_H
