
#include "CTrack.h"
#include "CGpx.h"

#include <QtGui>
#include <QtXml/QDomElement>

#if WIN32
#include <float.h>
#ifndef __MINGW32__
typedef __int32 int32_t;
#endif
#define isnan _isnan
#define FP_NAN NAN
#endif


const double WGS84_a = 6378137.0;
const double WGS84_b = 6356752.314245;
const double WGS84_f = 1.0/298.257223563;

#define RAD_TO_DEG	57.29577951308232
#define DEG_TO_RAD	.0174532925199432958


CTrack::CTrack()
    : mean(0)
    ,variance(0)
    ,sigma3(0)
    ,lessThan15m(0)
    ,lessThan10m(0)
    ,lessThan5m(0)
    ,offTrackCnt(0)
{

}

CTrack::~CTrack()
{

}

bool CTrack::load(QDomElement& gpxTrk)
{
    points.clear();

    QDomElement gpxName = gpxTrk.firstChildElement("name");
    if(gpxName.isElement())
    {
        name = gpxName.text();
    }


    QDomElement trkseg = gpxTrk.firstChildElement("trkseg");
    while(!trkseg.isNull())
    {
        QDomElement trkpt = trkseg.firstChildElement("trkpt");
        while (!trkpt.isNull())
        {
            pt_t pt;

            pt.lon = trkpt.attribute("lon").toDouble();
            pt.lat = trkpt.attribute("lat").toDouble();

            QDomElement gpxEle = trkpt.firstChildElement("ele");
            if(gpxEle.isElement())
            {
                pt.ele = gpxEle.text().toDouble();
            }

            QDomElement gpxTime = trkpt.firstChildElement("time");
            if(gpxTime.isElement())
            {
                int tzoffset;
                QString timetext = gpxTime.text();
                QDateTime timestamp = parseTimestamp(timetext, tzoffset);

                pt.timestamp = timestamp.toTime_t() - tzoffset;
                pt.timestamp_msec = timestamp.time().msec();

            }
            points << pt;

            trkpt = trkpt.nextSiblingElement("trkpt");
        }

        trkseg = trkseg.nextSiblingElement("trkseg");
    }

    QCryptographicHash md5(QCryptographicHash::Md5);
    md5.addData((char*)this, sizeof(CTrack));
    key = md5.result().toHex();

    addArtificialPoints();

    return !points.isEmpty();
}

void CTrack::save(QDomElement &gpxTrk, CGpx& gpx)
{
    QString str;

    QDomElement name = gpx.createElement("name");
    gpxTrk.appendChild(name);
    QDomText _name_ = gpx.createTextNode(getName());
    name.appendChild(_name_);

    QDomElement trkseg = gpx.createElement("trkseg");
    gpxTrk.appendChild(trkseg);

    foreach (const pt_t& pt, points)
    {
//        if(pt.distRef == 1e25)
//        {
//            continue;
//        }

        QDomElement trkpt = gpx.createElement("trkpt");
        trkseg.appendChild(trkpt);

//        str.sprintf("%1.8f", pt.latRef);
//        trkpt.setAttribute("lat",str);
//        str.sprintf("%1.8f", pt.lonRef);
//        trkpt.setAttribute("lon",str);

//        trkpt = gpx.createElement("trkpt");
//        trkseg.appendChild(trkpt);

        str.sprintf("%1.8f", pt.lat);
        trkpt.setAttribute("lat",str);
        str.sprintf("%1.8f", pt.lon);
        trkpt.setAttribute("lon",str);

    }
}

void CTrack::addArtificialPoints()
{
    int n,m;


    if(points.size() < 2)
    {
        return;
    }

    QVector<pt_t> newPoints;

    for(n = 1; n < points.size(); n++)
    {
        pt_t pt1 = points[n - 1];
        pt_t pt2 = points[n];

        int delta = pt2.timestamp - pt1.timestamp;

        newPoints << pt1;

        if(delta > 1)
        {
            double dx = pt2.lon - pt1.lon;
            double dy = pt2.lat - pt1.lat;

            double stepX = dx/delta;
            double stepY = dy/delta;

            for(m = 1; m < delta; m++)
            {
                pt_t p;
                p.lon = pt1.lon + m * stepX;
                p.lat = pt1.lat + m * stepY;
                p.ele = pt1.ele;
                p.timestamp = pt1.timestamp + m;
                p.timestamp_msec = pt1.timestamp_msec;

                newPoints << p;
            }
        }
    }

    newPoints << points.last();
    points = newPoints;

//    CGpx gpx(0);
//    QDomElement root = gpx.documentElement();
//    QDomElement gpxTrk = gpx.createElement("trk");
//    root.appendChild(gpxTrk);
//    save(gpxTrk, gpx);
//    gpx.save(getName() + "_points.gpx");

}


QDateTime CTrack::parseTimestamp(const QString &timetext, int& tzoffset)
{
    const QRegExp tzRE("[-+]\\d\\d:\\d\\d$");
    int i;

    tzoffset = 0;

    QString format = "yyyy-MM-dd'T'hh:mm:ss";

    i = timetext.indexOf(".");
    if (i != -1)
    {

        if(timetext[i+1] == '0')
        {
            format += ".zzz";
        }
        else
        {
            format += ".z";
        }
    }

    // trailing "Z" explicitly declares the timestamp to be UTC
    if (timetext.indexOf("Z") != -1)
    {
        format += "'Z'";
    }
    else if ((i = tzRE.indexIn(timetext)) != -1)
    {
        // trailing timezone offset [-+]HH:MM present
        // This does not match the original intentions of the GPX
        // file format but appears to be found occasionally in
        // the wild.  Try our best parsing it.

        // add the literal string to the format so fromString()
        // will succeed
        format += "'";
        format += timetext.right(6);
        format += "'";

        // calculate the offset
        int offsetHours(timetext.mid(i + 1, 2).toUInt());
        int offsetMinutes(timetext.mid(i + 4, 2).toUInt());
        if (timetext[i] == '-')
        {
            tzoffset = -(60 * offsetHours + offsetMinutes);
        }
        else
        {
            tzoffset = 60 * offsetHours + offsetMinutes;
        }
        tzoffset *= 60;          // seconds
    }

    QDateTime datetime = QDateTime::fromString(timetext, format);

    return datetime;
}

// from http://www.movable-type.co.uk/scripts/LatLongVincenty.html
// additional antipodal convergence trick might be a bit lame, but it seems to work
double CTrack::distance(const double lon1, const double lat1, const double lon2, const double lat2, double& a1, double& a2)
{
    double cosSigma = 0.0;
    double sigma = 0.0;
    double sinAlpha = 0.0;
    double cosSqAlpha = 0.0;
    double cos2SigmaM = 0.0;
    double sinSigma = 0.0;
    double sinLambda = 0.0;
    double cosLambda = 0.0;

    double L = lon2 - lon1;

    double U1 = atan((1-WGS84_f) * tan(lat1));
    double U2 = atan((1-WGS84_f) * tan(lat2));
    double sinU1 = sin(U1), cosU1 = cos(U1);
    double sinU2 = sin(U2), cosU2 = cos(U2);
    double lambda = L, lambdaP = (double)(2*M_PI);
    unsigned iterLimit = 20;

    while (fabs(lambda - lambdaP) > 1e-12)
    {
        if (!iterLimit)
        {
            lambda = M_PI;
            qDebug() << "No lambda convergence, most likely due to near-antipodal points. Assuming antipodal.";
        }

        sinLambda = sin(lambda);
        cosLambda = cos(lambda);
        sinSigma = sqrt((cosU2*sinLambda) * (cosU2*sinLambda) + (cosU1*sinU2-sinU1*cosU2*cosLambda) * (cosU1*sinU2-sinU1*cosU2*cosLambda));

        if (sinSigma==0)
        {
            return 0;            // co-incident points
        }

        cosSigma = sinU1 * sinU2 + cosU1 * cosU2 * cosLambda;
        sigma = atan2(sinSigma, cosSigma);
        sinAlpha = cosU1 * cosU2 * sinLambda / sinSigma;
        cosSqAlpha = 1 - sinAlpha * sinAlpha;
        cos2SigmaM = cosSigma - 2 * sinU1 * sinU2 / cosSqAlpha;

        if (isnan(cos2SigmaM))
        {
            cos2SigmaM = 0;      // equatorial line: cosSqAlpha=0 (6)
        }

        double C = WGS84_f/16 * cosSqAlpha * (4 + WGS84_f * (4 - 3 * cosSqAlpha));
        lambdaP = lambda;

        if (iterLimit--) lambda = L + (1-C) * WGS84_f * sinAlpha * (sigma + C*sinSigma*(cos2SigmaM + C * cosSigma * (-1 + 2 * cos2SigmaM * cos2SigmaM)));
    }

    double uSq = cosSqAlpha * (WGS84_a*WGS84_a - WGS84_b*WGS84_b) / (WGS84_b*WGS84_b);
    double A = 1 + uSq/16384*(4096+uSq*(-768+uSq*(320-175*uSq)));
    double B = uSq/1024 * (256+uSq*(-128+uSq*(74-47*uSq)));
    double deltaSigma = B*sinSigma*(cos2SigmaM+B/4*(cosSigma*(-1+2*cos2SigmaM*cos2SigmaM)-B/6*cos2SigmaM*(-3+4*sinSigma*sinSigma)*(-3+4*cos2SigmaM*cos2SigmaM)));
    double s = WGS84_b*A*(sigma-deltaSigma);

    a1 = atan2(cosU2 * sinLambda, cosU1 * sinU2 - sinU1 * cosU2 * cosLambda) * 360 / (2*M_PI);
    a2 = atan2(cosU1 * sinLambda, -sinU1 * cosU2 + cosU1 * sinU2 * cosLambda) * 360 / (2*M_PI);
    return s;
}

void CTrack::setReference(CTrack& ref)
{
    int n,m,j15 = 0, j10 = 0, j5 = 0;
    // for all track points

    for(n = 0; n < points.size(); n++)
    {
        double lon = 0, lat = 0;
        double dist = 1e25;
        pt_t& pt = points[n];


        // search shortest distance to reference track
        for(m = 1; m < ref.points.size(); m++)
        {
            pt_t p1 = ref.points[m - 1];
            pt_t p2 = ref.points[m];

            double dx = p2.lon - p1.lon;
            double dy = p2.lat - p1.lat;

            double d_p1_p2 = sqrt(dx * dx + dy * dy);

            double u = ((pt.lon - p1.lon) * dx + (pt.lat - p1.lat) * dy) / (d_p1_p2 * d_p1_p2);


            if(u < 0)
            {
                continue;
            }

            double a1,a2,dist_,lon_,lat_;
            if(u <= 1.0)
            {
                lon_  = p1.lon + u * dx;
                lat_  = p1.lat + u * dy;
            }
            else
            {
                lon_  = p2.lon;
                lat_  = p2.lat;
            }

            dist_ = distance(lon_ * DEG_TO_RAD,lat_ * DEG_TO_RAD, pt.lon * DEG_TO_RAD, pt.lat * DEG_TO_RAD, a1, a2);
            if(dist_ < dist)
            {
                dist = dist_;
                lon  = lon_;
                lat  = lat_;
            }
        }


        pt.distRef = dist;
        pt.lonRef  = lon;
        pt.latRef  = lat;
    }

    n        = 0;
    m        = 0;
    mean     = 0;
    variance = 0;

    foreach(const pt_t& pt, points)
    {
        if(pt.distRef > 30)
        {
            offTrackCnt++;
        }

        if(pt.distRef < 15)
        {
            j15++;
        }
        if(pt.distRef < 10)
        {
            j10++;
        }
        if(pt.distRef < 5)
        {
            j5++;
        }


        if(pt.distRef <= 30)
        {
            mean += pt.distRef;
            m++;
        }
        n++;

    }

    mean = mean/m;

    foreach(const pt_t& pt, points)
    {
        if(pt.distRef > 30)
        {
            continue;
        }
        variance += (pt.distRef - mean) * (pt.distRef - mean);
    }
    variance    = variance/n;
    sigma3      = 3*sqrt(variance);
    lessThan15m = j15 * 100.0/n;
    lessThan10m = j10 * 100.0/n;
    lessThan5m  = j5  * 100.0/n;
}



