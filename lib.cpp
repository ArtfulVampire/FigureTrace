#include "lib.h"

#include <fstream>
#include <cmath>
#include <map>
#include <valarray>
#include <queue>
#include <utility>

#include <QPainter>

std::ostream & operator<< (std::ostream & os, const QPoint & in)
{
	os << in.x() << "\t" << in.y();
	return os;
}

std::ostream & operator<< (std::ostream & os, const QString & in)
{
	os << in.toStdString();
	return os;
}

std::ostream & operator<< (std::ostream & os, const QPointF & in)
{
	os << in.x() << "\t" << in.y();
	return os;
}

std::ostream & operator<< (std::ostream & os, const direction & in)
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
	/// should never get here, no return would cause a crash
}
direction dirFromInt(int in)
{
	return static_cast<direction>((in + 8) % 8);
}

double myRound(double in, int decims)
{
	return std::round(in * std::pow(10., decims)) / std::pow(10., decims);
}

double quality(double in, double low, double hig)
{
	return (hig - in) / (hig - low) * 100.;
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

	return std::min(res1, res2);
}
template double trackingQuality(const std::vector<QPoint> & fig,
								const std::vector<QPoint> & track);
template double trackingQuality(const std::vector<QPointF> & fig,
								const std::vector<QPointF> & track);

template <typename pointType>
double trackingQualityInner(const std::vector<pointType> & fig,
							const std::vector<pointType> & alignedTrack)
{
	const int windowSize = 40;
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
	return ret;
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
	return res;
}
template std::vector<QPoint> smoothCurve(const std::vector<QPoint> & curve);
template std::vector<QPointF> smoothCurve(const std::vector<QPointF> & curve);


template <typename pointType>
std::vector<pointType>
adjustCurve(const std::vector<pointType> & curve,
			const QSize & size,
			bool dontEnlarge)
{
	/// [x/y], [min/max]
	decltype(curve[0].x()) mins[2][2] = {{curve[0].x(), curve[0].x()},
										 {curve[0].x(), curve[0].y()}};
	for(auto & in : curve)
	{
		mins[0][0] = std::min(mins[0][0], in.x());
		mins[0][1] = std::max(mins[0][1], in.x());
		mins[1][0] = std::min(mins[1][0], in.y());
		mins[1][1] = std::max(mins[1][1], in.y());
	}
	const pointType center((mins[0][0] + mins[0][1]) / 2.,
			(mins[1][0] + mins[1][1]) / 2.);

	double scaling = std::min(
						 (size.width() - 40) / double(mins[0][1] - mins[0][0]),
			(size.height() - 40) / double(mins[1][1] - mins[1][0]));

	if(scaling > 1. && dontEnlarge) { scaling = 1.; }

	/// test cout
//	std::cout << mins[0][0] << std::endl;
//	std::cout << mins[0][1] << std::endl;
//	std::cout << mins[1][0] << std::endl;
//	std::cout << mins[1][1] << std::endl;
//	std::cout << center << "\t" << scaling << std::endl;

	std::vector<pointType> res{curve};
	for(auto & in : res)
	{
		in = pointType(size.width(), size.height()) / 2 /// new center
			 + (in - center) * scaling;
	}
	return res;
}
template std::vector<QPoint> adjustCurve(const std::vector<QPoint> & curve,
										 const QSize & size,
										 bool dontEnlarge);
template std::vector<QPointF> adjustCurve(const std::vector<QPointF> & curve,
										  const QSize & size,
										  bool dontEnlarge);


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

QPixmap drawFigure(const std::vector<QPoint> & in, const QSize & size, int lineWidth)
{
	QPixmap res(size);
	res.fill("black");
	QPainter paint;
	paint.begin(&res);
	paint.setPen(QPen(QBrush("white"), lineWidth));

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


std::list<std::list<int>> generateDirectionVariants(int numSteps)
{
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
			if(std::abs(rot) >= 3) { flag = false; break; }
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
	for(int i : {-2, 2})
	{
		for(auto in : directionVariantsLittle)
		{
			in.push_front(i);
			if(std::abs(std::accumulate(std::begin(in), std::end(in), 0)) >= 4) continue;
			directionVariants.push_back(in);
		}
	}
	return directionVariants;
}


std::vector<QPoint> readFromPicture(const QString & picPath,
									int lineThickness,
									int numSteps,
									int stepSize)
{
	/// new version 29-Aug-18

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


	/// lowest in the rightest column
	const QPoint curveS(
				std::get<1>(colorMap[lineLightness]),
				std::get<2>(colorMap[lineLightness]));

	QPoint curveStart = curveS - QPoint(lineThickness / 2, 0);
	auto curveColor = pic.pixelColor(curveStart);

	std::list<QPoint> res{};
//	res.reserve(colorVec[lineLightness] / lineThickness * 2); /// heuristic
	res.push_back(curveStart);

	/// prepare a list of possible search paths



	/// follow the curve
	const int maxHistoryPointsSize = 49;			/// magic const
	const int maxHistoryDirsSize = 4;			/// magic const
	std::list<direction> historyDirs{};		/// what were the last N movements
	std::list<QPoint> historyPoints{};		/// what were the last N movements
	int counter = 0;

	direction prevDir(static_cast<direction>(6));	/// initial direction south-west
	std::vector<std::pair<direction, int>> tmp{};	/// stores neighbour points quality

	std::list<std::list<int>> directionVariants = generateDirectionVariants(numSteps);

	QPoint currPoint{curveStart};
	while(1)
	{
		bool fastFlag = false;
		for(auto & variant : directionVariants)
		{
			int badPoints{0};
			std::vector<QPoint> nextPoints{};
			nextPoints.push_back(currPoint);

			direction tmpDir{prevDir};
			bool isGood{true};
			for(int dir : variant)
			{
				tmpDir = tmpDir + dir;
				nextPoints.push_back(nextPoints.back() + stepSize * tmpDir);

				if(contains(historyPoints, nextPoints.back()))
				{
//					std::cout << "history collision " << nextPoints.back() << std::endl;
					isGood = false;
					break;
				}

				int badPt = numPointsMask(pic, nextPoints.back(), curveColor, lineThickness);

				/// test cout
//				if(currPoint == QPoint{1659, 1511})
//				{
//					std::cout
//							<< "intDir = " << dir << "\t\t"
//							<< "pointDir = " << getDs(tmpDir) << "\t"
//							<< "nextPoint = " << nextPoints.back() << "\t"
//							<< "quality = " << badPt << "\t"
//							<< std::endl;
//				}

				badPoints += badPt;
			}
			/// test cout
//			if(currPoint == QPoint{1659, 1511})
//			{
//				std::cout << std::endl;
//			}
			if(!isGood)
			{
				/* nothing to clear */
				continue;
			}
			tmp.push_back({prevDir + variant.front(), badPoints});

			/// add all the path if it has perfect fit
			/// TO DO - add history processing
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
			std::cout << "too bad " << tmp[0].second << std::endl;
			exit(0);
		}


		std::cout
				<< currPoint << "\t"
				<< static_cast<int>(tmp[0].first) << std::endl
				<< currPoint + stepSize * tmp[0].first << std::endl
				<< tmp[0].second << std::endl
				<< std::endl;


		prevDir = tmp[0].first;

		historyPoints.push_back(currPoint);
		historyPoints.push_back(currPoint + stepSize * (prevDir - 2));
		historyPoints.push_back(currPoint + stepSize * (prevDir + 2));
		historyPoints.push_back(currPoint + stepSize * (prevDir - 1));
		historyPoints.push_back(currPoint + stepSize * (prevDir + 1));
		historyPoints.push_back(currPoint + stepSize * (prevDir + 2) + stepSize * (prevDir + 3));
		historyPoints.push_back(currPoint + stepSize * (prevDir - 2) + stepSize * (prevDir - 3));
		if(historyPoints.size() > maxHistoryPointsSize)
		{
			historyPoints.pop_front();
			historyPoints.pop_front();
			historyPoints.pop_front();
			historyPoints.pop_front();
			historyPoints.pop_front();
			historyPoints.pop_front();
			historyPoints.pop_front();
		}

		currPoint += stepSize * prevDir;
		res.push_back(currPoint);
		tmp.clear();

		historyDirs.push_back(prevDir);
		if(historyDirs.size() > maxHistoryDirsSize)
		{
			historyDirs.pop_front();
			prevDir = dirFromPoint((QPointF(std::accumulate(std::begin(historyDirs),
															std::end(historyDirs),
															QPoint()))
									/ maxHistoryDirsSize).toPoint());
		}

		if(counter > 50)
		{
			static const int nnn = 15;

			static auto ref{std::begin(res)};
			if(ref == std::begin(res))
			{

				for(int i = 0; i < nnn; ++i)
				{
					++ref;
				}
			}

			if(QPoint::dotProduct(res.back() - (*ref),
								  res.back() - (*ref)) <= std::pow(lineThickness / 2, 2)

			   )
			{
				res.push_back((res.back() + *ref) / 2);
				for(int i = 0; i < nnn; ++i) { res.pop_front(); }
				break;
			}
		}
		++counter;
	}
	std::vector<QPoint> res2(res.size());
	std::copy(std::begin(res), std::end(res), std::begin(res2));
	return res2;
}

std::vector<QPoint> readFromPictureSimple(const QString & picPath)
{
	/// old version 29-Aug-18

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
	/// lowest in the rightest column
	const QPoint curveStart(
				std::get<1>(colorMap[lineLightness]),
				std::get<2>(colorMap[lineLightness]));
	auto curveColor = pic.pixelColor(curveStart);


	std::vector<QPoint> res{};
	res.reserve(colorVec[lineLightness] * 2);
	res.push_back(curveStart);

	QPoint currPoint = curveStart;
	direction prevDir{}; /// = history.back()
	/// detect initial direction
	/// check directions (NW, SW, WW, NN), diagonal first. other are empty
	for(int d : {0, 6, 7, 1})
	{
		direction dir = static_cast<direction>(d);
		QPoint nextPoint = curveStart + getDs(dir);
		if(areColorsSimilar(pic.pixelColor(nextPoint), curveColor))
		{
			res.push_back(nextPoint);
			currPoint = nextPoint;
			prevDir = dir;
			break;
		}
	}

	/// follow the curve
	const int maxHistorySize = 10;		/// magic const
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
			if(areColorsSimilar(pic.pixelColor(nextPoint), curveColor))
			{
//				std::cout
//						<< nextPoint << "\t"
//						<< QPoint::dotProduct(nextPoint - curveStart, nextPoint - curveStart)
//						<< std::endl;
				res.push_back(nextPoint);
				history.push(dir);
				if(history.size() > maxHistorySize) { history.pop(); }
				currPoint = nextPoint;
				prevDir = dir;
				break;
			}
		}
		if(QPoint::dotProduct(nextPoint - curveStart,
							  nextPoint - curveStart) <= 4
		   && counter > 30
		   )
		{
			res.push_back((nextPoint + curveStart) / 2);
			break;
		}
		++counter;
	}
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


template <typename Cont, typename Val>
bool contains(const Cont & cont, Val val)
{
	return std::find(std::begin(cont), std::end(cont), val) != std::end(cont);
}
template bool contains(const std::list<direction> & cont, direction val);
template bool contains(const std::list<QPoint> & cont, QPoint val);
