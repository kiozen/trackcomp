#ifndef CTRACK_H
#define CTRACK_H

#include <QVector>
#include <QString>
#include <QDateTime>
class CGpx;
class QDomElement;

class CTrack
{
    public:
        CTrack();
        ~CTrack();

        bool load(QDomElement &gpxTrk);

        void save(QDomElement &gpxTrk, CGpx &gpx);

        const QString& getKey(){return key;}

        const QString& getName(){return name;}


        void setReference(CTrack& ref);

        double getMean(){return mean;}
        double getVariance(){return variance;}
        double get3Sigma(){return sigma3;}
        double getLessThan15m(){return lessThan15m;}
        double getLessThan10m(){return lessThan10m;}
        double getLessThan5m(){return lessThan5m;}
        int getOffTrackCnt(){return offTrackCnt;}

    private:
        void addArtificialPoints();
        QDateTime parseTimestamp(const QString &timetext, int& tzoffset);
        double distance(const double lon1, const double lat1, const double lon2, const double lat2, double& a1, double& a2);

        struct pt_t
        {
            pt_t() : lon(0), lat(0), ele(0), timestamp(0), timestamp_msec(0), distRef(1e25){}
            double lon;
            double lat;
            double ele;
            quint32 timestamp;
            quint32 timestamp_msec;

            double distRef;
            double lonRef;
            double latRef;


        };

        QString key;
        QString name;
        QVector<pt_t> points;

        double mean;
        double variance;
        double sigma3;
        double lessThan15m;
        double lessThan10m;
        double lessThan5m;
        int offTrackCnt;
};




#endif //CTRACK_H
