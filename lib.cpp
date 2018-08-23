#include "lib.h"

#include <fstream>
#include <cmath>

#include <QPainter>

std::ostream & operator << (std::ostream & os, const QPoint & in)
{
	os << in.x() << "\t" << in.y();
	return os;
}

template <typename pointType>
double trackingQuality(const std::vector<pointType> & fig,
					   const std::vector<pointType> & track)
{
	auto appTrack = approximateCurve(track);
	auto localTrack = alignTracking(fig, appTrack);
	saveFigure("/media/Files/Data/Tracking/tr1.txt", localTrack);
	double res1 = trackingQualityInner(fig, localTrack);

	std::reverse(std::begin(localTrack) + 1, std::end(localTrack));
	saveFigure("/media/Files/Data/Tracking/tr2.txt", localTrack);
	double res2 = trackingQualityInner(fig, localTrack);

//	std::cout << res1 << "\t" << res2 << std::endl;
	return std::max(res1, res2);
}
template double trackingQuality(const std::vector<QPoint> & fig,
								const std::vector<QPoint> & track);
template double trackingQuality(const std::vector<QPointF> & fig,
								const std::vector<QPointF> & track);

template <typename pointType>
double trackingQualityInner(const std::vector<pointType> & fig,
							const std::vector<pointType> & alignedTrack)
{
	const int windowSize = 30;
	std::vector<double> res{};
	int prevIndex = 0;
	for(const auto & figPoint : fig)
	{
		double dist = 500.;
		for(int i = prevIndex; i < std::min(prevIndex + windowSize, int(alignedTrack.size())); ++i)
		{
			pointType a = figPoint - alignedTrack[i];
			double d = std::sqrt(pointType::dotProduct(a, a));
			if(d < dist)
			{
				dist = d;
				prevIndex = i + 1;
			}

			if(dist < 1							/// good fit
			   || (d > dist * 3. && dist < 5)	/// were close and now too far
			   )
			{
				break;
			}

		}
		res.push_back(dist);
	}
	/// average distance of best fit
	const double ret = std::accumulate(std::begin(res), std::end(res), 0.) / res.size();
//	std::cout << "ret = " << ret << std::endl;

	/// mapped [lowest-highest] -> [100-0]
	const double lowest = 2;
	const double highest = 20;
	return (highest - ret) / (highest - lowest) * 100.;
}
template double trackingQualityInner(const std::vector<QPoint> & fig,
									const std::vector<QPoint> & alignedTrack);
template double trackingQualityInner(const std::vector<QPointF> & fig,
									const std::vector<QPointF> & alignedTrack);
template <typename pointType>
std::vector<pointType>
alignTracking(const std::vector<pointType> & fig,
			  const std::vector<pointType> & track)
{
	double dist = 500.; // magic constant "very many"
	int index = 0;
	for(int i = 0; i < track.size(); ++i)
	{
		auto a = track[i] - fig[0];
		double d = std::sqrt(pointType::dotProduct(a, a));
		if(d < dist)
		{
			index = i;
			dist = d;
		}
	}

	/// same direction
	std::vector<pointType> res1{track.size()};
	std::copy(std::begin(track) + index, std::end(track), std::begin(res1));
	std::copy(std::begin(track), std::begin(track) + index,
			  std::begin(res1) + track.size() - index);
	return res1;
}
template
std::vector<QPoint>
alignTracking(const std::vector<QPoint> & fig,
				const std::vector<QPoint> & track);
template
std::vector<QPointF>
alignTracking(const std::vector<QPointF> & fig,
				const std::vector<QPointF> & track);

std::vector<QPoint> loadFigure(const QString & filePath)
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

QPixmap drawFigure(const std::vector<QPoint> & in)
{
	QPixmap res(picSiz);
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

template <typename pointType>
void saveFigure(const QString & filePath, const std::vector<pointType> & in)
{
	std::ofstream outStr(filePath.toStdString());
	for(const pointType & p : in)
	{
		outStr << p.x() << "\t" << p.y() << "\n";
	}
	outStr.close();
}
template
void saveFigure(const QString & filePath, const std::vector<QPoint> & in);
template
void saveFigure(const QString & filePath, const std::vector<QPointF> & in);

template <typename pointType>
std::vector<pointType> approximateCurve(const std::vector<pointType> & in)
{
	std::vector<pointType> res{};
	res.reserve(in.size() * 3); // heuristic

	for(auto it = std::begin(in); it != std::end(in); ++it)
	{
		auto next = it + 1;
		if(next == std::end(in)) { next = std::begin(in); } /// make "a cirle"
		res.push_back(*it);
		double x1 = (*it).x();
		double y1 = (*it).y();
		double x2 = (*next).x();
		double y2 = (*next).y();
		double diffX = x2 - x1;
		double diffY = y2 - y1;
		int steps = std::max(std::abs(diffX), std::abs(diffY));

		for(int i = 1; i < steps; ++i)
		{
			res.push_back(pointType(x1 + i * diffX / steps,
									y1 + i * diffY / steps));
		}
	}
	return res;
}
template std::vector<QPoint> approximateCurve(const std::vector<QPoint> & in);
template std::vector<QPointF> approximateCurve(const std::vector<QPointF> & in);