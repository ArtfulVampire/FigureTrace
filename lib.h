#ifndef LIB_H
#define LIB_H

#include <iostream>
#include <vector>

#include <QPixmap>
#include <QPoint>

const QSize picSiz(650, 650);

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

std::vector<QPoint> loadFigure(const QString & filePath);

template <typename pointType>
std::vector<pointType> approximateCurve(const std::vector<pointType> & in);

QPixmap drawFigure(const std::vector<QPoint> & in);

template <typename pointType>
void saveFigure(const QString & filePath, const std::vector<pointType> & in);

#endif // LIB_H