#include "lib.h"

#include <fstream>
#include <cmath>
#include <map>
#include <valarray>
#include <queue>
#include <utility>

#include <QPainter>

std::ostream & operator << (std::ostream & os, const QPoint & in)
{
	os << in.x() << "\t" << in.y();
	return os;
}

std::ostream & operator << (std::ostream & os, const direction & in)
{
	os << getDs(in);
	return os;
}

direction operator++(direction & in, int)
{
	return in = direction((static_cast<int>(in) + 1) % 8);
}

direction operator+(const direction & in, int a)
{
	return direction((static_cast<int>(in) + a + 8) % 8); /// +8 for "0 + (-1)"
}

QPoint operator*(const direction & in, int a)
{
	return getDs(in) * a;
}

QPoint operator*(int a, const direction & in)
{
	return getDs(in) * a;
}

QPoint operator+(const direction & a, const direction & b)
{
	return getDs(a) + getDs(b);
}

QPoint operator+(const direction & a, const QPoint & b)
{
	return getDs(a) + b;
}

QPoint operator+(const QPoint & a, const direction & b)
{
	return a + getDs(b);
}

direction operator-(const direction & in, int a)
{
	return direction((static_cast<int>(in) - a + 8) % 8);
}

direction operator--(direction & in, int)
{
	return in = direction((static_cast<int>(in) - 1 + 8) % 8);
}

direction opposite(const direction & in)
{
	return direction((static_cast<int>(in) + 4) % 8);
}

QPoint getDs(const direction & in)
{
	switch(in)
	{
	case direction::NW: { return {-1, -1}; }
	case direction::NN: { return {0, -1}; }
	case direction::NE: { return {1, -1}; }
	case direction::EE: { return {1, 0}; }
	case direction::SE: { return {1, 1}; }
	case direction::SS: { return {0, 1}; }
	case direction::SW: { return {-1, 1}; }
	case direction::WW: { return {-1, 0}; }
	default: { /* never get here */ return {0, 0}; }
	}
}

direction dirFromPoint(const QPoint & pt)
{
	if(		pt == QPoint{-1, -1})	{ return direction::NW; }
	else if(pt == QPoint{0, -1})	{ return direction::NN; }
	else if(pt == QPoint{1, -1})	{ return direction::NE; }
	else if(pt == QPoint{1, 0})		{ return direction::EE; }
	else if(pt == QPoint{1, 1})		{ return direction::SE; }
	else if(pt == QPoint{0, 1})		{ return direction::SS; }
	else if(pt == QPoint{-1, 1})	{ return direction::SW; }
	else if(pt == QPoint{-1, 0})	{ return direction::WW; }
}
direction dirFromInt(int in)
{
	return static_cast<direction>((in + 8) % 8);
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

template <typename pointType>
std::vector<pointType>
smoothCurve(const std::vector<pointType> & curve)
{
	std::vector<pointType> res{curve};

	const int halfWidth = 4; /// to do with gaussian, 3 * sigma = halfWidth
	std::valarray<double> gaus{2 * halfWidth + 1};
	for(int i = 0; i < gaus.size(); ++i)
	{
		double x = (halfWidth - i) * 3. / (halfWidth + 1);
		gaus[i] = std::exp(- x * x / 2.);
	}
	gaus /= gaus.sum();


	std::vector<pointType> tmp(2 * halfWidth + 1); /// for "in-place" smoothing
	for(int i = 0; i < halfWidth; ++i)
	{
		tmp[i] = res[i];
	}

	for(int i = halfWidth; i < res.size() - halfWidth; ++i)
	{
		tmp[halfWidth] = res[i];
		std::copy(std::begin(res) + i + 1,
				  std::begin(res) + i + halfWidth + 1,
				  std::begin(tmp) + halfWidth);
		res[i] = std::inner_product(std::begin(tmp),
									std::end(tmp),
									std::begin(gaus),
									pointType(0, 0));

		for(int j = 0; j < halfWidth; ++j)
		{
			tmp[j] = tmp[j + 1];
		}
	}
	std::cout << "end2" << std::endl;
	return res;
}
template std::vector<QPoint> smoothCurve(const std::vector<QPoint> & curve);
template std::vector<QPointF> smoothCurve(const std::vector<QPointF> & curve);

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

QImage thresholding(const QString & picPath)
{
	QImage pic(picPath);
	for(int x = 0; x < pic.width(); ++x)
	{
		for(int y = 0; y < pic.height(); ++y)
		{
			if(pic.pixelColor(x, y).lightnessF() > 0.5) { pic.setPixelColor(x, y, QColor("white"));}
			else { pic.setPixelColor(x, y, QColor("black"));}
		}
	}
	return pic;
}

QImage makeThinnerLine(const QString & picPath, bool isLineWhite, int num)
{
	const QColor curveColor = isLineWhite ? QColor("white") : QColor("black");
	const QColor bgColor	= isLineWhite ? QColor("black") : QColor("white");
	std::vector<QPoint> toExclude{};

	QImage pic(picPath);
	for(int j = 0; j < num; ++j)
	{
		for(int x = 0; x < pic.width(); ++x)
		{
			for(int y = 0; y < pic.height(); ++y)
			{
				QPoint p(x, y);
				if(!areColorsSimilar(pic.pixelColor(p), curveColor)) /// not the curve color
				{
					continue;
				}

				for(int i = 0; i < 8; ++i)
				{
					direction dir(static_cast<direction>(i));
					if(areColorsSimilar(pic.pixelColor(p + getDs(dir)), bgColor))
					{
						toExclude.push_back(p);
						break;
					}
				}
			}
		}
		std::cout << toExclude.size() << std::endl;
		for(auto pt : toExclude)
		{
			pic.setPixelColor(pt, bgColor);
		}
		toExclude.clear();
	}
	return pic;
}

bool areColorsSimilar(const QColor & in1, const QColor & in2, int thr)
{
	return (std::abs(in1.red() - in2.red())
			+ std::abs(in1.green() - in2.green())
			+ std::abs(in1.blue() - in2.blue())) < thr;
}

int numPointsMask(const QImage & pic, const QPoint & pt,
				  const QColor & curveColor, int lineThickness)
{
	int res{0};
	for(int x = -lineThickness / 2; x <= lineThickness / 2; ++x)
	{
		for(int y = -lineThickness / 2; y <= lineThickness / 2; ++y)
		{
			if(std::abs(std::pow(x, 2) + std::pow(y, 2))
			   <= std::pow(lineThickness / 2., 2)
			   && !areColorsSimilar(pic.pixelColor(pt - QPoint(x, y)), curveColor))
			{
				++res;
			}
		}
	}
	return res;
}

std::vector<QPoint> readFromPicture(const QString & picPath)
{
	QImage pic(picPath);
	/// detect background and curve colors
	/// colorMap[ligthness] -> (number, x, y)
	std::map<double, std::tuple<int, int, int>> colorMap{};
	for(int x = 0; x < pic.width(); ++x)
	{
		for(int y = 0; y < pic.height(); ++y)
		{
			auto index = pic.pixelColor(x, y).lightnessF();
			std::get<0>(colorMap[index]) += 1;
			std::get<1>(colorMap[index]) = x;
			std::get<2>(colorMap[index]) = y;
		}
	}

	/// [lightness] = count;
	std::vector<int> colorVec(2); /// 0 - dark, 1 - light
	for(const auto & in : colorMap)
	{
		colorVec[ in.first > 0.5 ] += std::get<0>(in.second);
	}
	int lineLightness = colorVec[0] > colorVec[1];

#if 01
	/// new version 29-Aug-18

	/// lowest in the rightest column
	const QPoint curveS(
				std::get<1>(colorMap[lineLightness]),
				std::get<2>(colorMap[lineLightness]));

	/// the line should be vertical here

	int lineThickness{};
	for(int i = 1; i < 50; ++i) /// magic const
	{
		if(!areColorsSimilar(pic.pixelColor(curveS - QPoint(i, 0)),
						   pic.pixelColor(curveS)))
		{
			lineThickness = i;
			break;
		}
	}




	lineThickness = 16;




	const QPoint curveStart = curveS - QPoint(lineThickness / 2, 0);
	auto curveColor = pic.pixelColor(curveStart);

	std::vector<QPoint> res{};
	res.reserve(colorVec[lineLightness] / lineThickness * 2); /// heuristic
	res.push_back(curveStart);


	/// follow the curve
	const int maxHistorySize = 25;			/// magic const
	std::list<direction> historyDirs{};		/// what were the last N movements
	int counter = 0;


	const int numSteps = 4;
	const int base = 3;
	std::list<std::list<int>> directionVariantsLittle{};
	for(int i = 0; i < std::pow(base, numSteps); ++i)
	{
		bool flag = true;
		std::list<int> tmp{};
		int rot{0};
		for(int pos = 0; pos < numSteps; ++pos)
		{
			int a = (i / int(std::pow(base, pos))) % base;
			if(a == 2) { a = -1; }
			tmp.push_front(a);
			rot += a;
			if(std::abs(rot) >= 4) { flag = false; break; }
		}
		if(flag)
		{
			directionVariantsLittle.push_back(tmp);
		}
	}

	std::list<std::list<int>> directionVariants{};
	for(int i : {0, 1, -1})
	{
		for(auto in : directionVariantsLittle)
		{
			in.push_front(i);
			directionVariants.push_back(in);
		}
	}
	for(int i : {0, 1, -1})
	{
		for(auto in : directionVariantsLittle)
		{
			in.push_front(i);
			if(std::abs(std::accumulate(std::begin(in), std::end(in), 0)) >= 4) continue;
			directionVariants.push_back(in);
		}
	}


//	for(auto & in1 : directionVariants)
//	{
//		for(auto in2 : in1)
//		{
//			std::cout << in2;
//		}
//		std::cout << std::endl;
//	}
//	std::cout << std::endl; exit(0);




//	int abc{0};
//	std::vector<QPoint> mmm
//	{
//		{1652, 1527},
//		{1654, 1525},
//		{1656, 1523},
//		{1658, 1521},
//		{1658, 1519},
//		{1658, 1517},
////		{1658, 1515},
//	};
//	for(auto p : mmm)
//	{
//		abc += numPointsMask(pic, p, curveColor, lineThickness);
//	}
//	std::cout << abc << std::endl; exit(0);




	QPoint currPoint{curveStart};
	direction prevDir(static_cast<direction>(0));	/// initial direction north-east
	std::vector<std::pair<direction, int>> tmp{};	/// stores neighbour points quality

	const int stepSize = 2;

	std::cout << "start = " << curveStart << std::endl;

	while(1)
	{
		bool fastFlag = false;
		for(auto & variant : directionVariants)
		{
			if(currPoint == QPoint{1652, 1527} && variant == std::list<int>{0, 0, 0, -1, 0})
			{
				std::cout << "kjasdasdjfn" << std::endl;
			}

			int badPoints{0};
			std::vector<QPoint> nextPoints{};
			nextPoints.push_back(currPoint);

			direction tmpDir{prevDir};
			for(int dir : variant)
			{
				tmpDir = tmpDir + dir;
				nextPoints.push_back(nextPoints.back() + stepSize * tmpDir);
				badPoints += numPointsMask(pic, nextPoints.back(), curveColor, lineThickness);
			}
			tmp.push_back({prevDir + variant.front(), badPoints});

			if(currPoint == QPoint{1652, 1527} && variant == std::list<int>{0, 0, 0, -1, 0})
			{
				std::cout << "bad points = " << badPoints << std::endl;
			}

//			if(badPoints == 0)
//			{
//				for(const QPoint & in : nextPoints)
//				{
//					res.push_back(in);
//				}
//				currPoint = res.back();
//				prevDir = tmpDir;
//				fastFlag = true;
//				break;
//			}
		}
		if(fastFlag)
		{
			tmp.clear();
			continue;
		}



		std::stable_sort(std::begin(tmp), std::end(tmp),
				  [](const auto & in1, const auto & in2)
		{
			return in1.second < in2.second;
		});



		if(tmp[0].second > 60)
		{
//			for(auto & in : tmp)
//			{
//				std::cout << in.first << "\t" << in.second << std::endl;
//			}
			std::cout << "too bad " << tmp[0].second << std::endl;
			exit(0);
		}


		std::cout
				<< currPoint << "\t" << static_cast<int>(tmp[0].first) << std::endl
//				<< currPoint + stepSize * tmp[0].first << std::endl
				<< tmp[0].second << std::endl
				<< std::endl;

		currPoint += stepSize * tmp[0].first;
		res.push_back(currPoint);
		prevDir = tmp[0].first;
		tmp.clear();

		historyDirs.push_back(prevDir);
		if(historyDirs.size() > maxHistorySize)
		{
			historyDirs.pop_front();
//			prevDir = dirFromPoint((QPointF(std::accumulate(std::begin(historyDirs),
//															std::end(historyDirs),
//															QPoint()))
//									/ maxHistorySize).toPoint());
//			std::cout << prevDir << std::endl;
		}

		if(QPoint::dotProduct(res.back() - curveStart,
							  res.back() - curveStart) <= std::pow(lineThickness / 2, 2)
		   && counter > lineThickness * 5
		   )
		{
			std::cout << "finish" << std::endl;
			res.push_back((res.back() + curveStart) / 2);
			break;
		}
		++counter;
	}
	return res;
#endif

#if 0
	/// old version 29-Aug-18
	/// lowest in the rightest column
	const QPoint curveStart(
				std::get<1>(colorMap[lineLightness]),
				std::get<2>(colorMap[lineLightness]));
//	std::cout << "start point = " << curveStart << std::endl;
	auto curveColor = pic.pixelColor(curveStart);



	std::vector<QPoint> res{}; res.reserve(colorVec[lineLightness]);
	res.push_back(curveStart);

	QPoint currPoint = curveStart;
	direction prevDir; /// = history.back()
	/// detect initial direction
	/// at first check diagonal directions (NW, SW, WW, NN), other are empty
	for(int d : {0, 6, 7, 1})
	{
		direction dir = static_cast<direction>(d);
		QPoint nextPoint = curveStart + getDs(dir);
		if(areCloseEnough(pic.pixelColor(nextPoint), curveColor))
		{
			res.push_back(nextPoint);
			currPoint = nextPoint;
			prevDir = dir;
			break;
		}
	}

	/// follow the curve
	const int maxHistorySize = 25;		/// magic const
	std::queue<direction> history{};	/// what were the last N movements
	int counter = 0;

	while(1)
	{
		QPoint nextPoint{};
		for(int num : {
			0,			/// same direction
			+1, -1,		/// a little curvy
			+2, -2,		/// orthogonal
//			+3, -3,		/// acute
//			+4			/// go back?
	})
		{
			/// add curve vector history
			/// add comparison between left- and right-side turns - which is better
			direction dir(prevDir + num);
			nextPoint = currPoint + getDs(dir);
			if(areCloseEnough(pic.pixelColor(nextPoint), curveColor))
			{
				std::cout
						<< nextPoint << "\t"
//						<< QPoint::dotProduct(nextPoint - curveStart, nextPoint - curveStart)
						<< std::endl;
				res.push_back(nextPoint);
				history.push(dir);
				if(history.size() > maxHistorySize) { history.pop(); }
				currPoint = nextPoint;
				prevDir = dir;
				break;
			}
		}
		if(QPoint::dotProduct(nextPoint - curveStart,
							  nextPoint - curveStart) <= 256 /// sqr(lineWidth)
		   && counter > 30
		   )
		{
			res.push_back((nextPoint + curveStart) / 2);
			break;
		}
		++counter;
	}
	return res;
#endif
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


template <typename Cont, typename Val>
bool contains(const Cont & cont, Val val)
{
	return std::find(std::begin(cont), std::end(cont), val) != std::end(cont);
}
template bool contains(const std::list<direction> & cont, direction val);
template bool contains(const std::list<QPoint> & cont, QPoint val);
