#ifndef __VG_SUPPORT_POLYLINE_H__
#define __VG_SUPPORT_POLYLINE_H__

#include "MapAbstractItem.h"
#include <QGeoCoordinate>
#include <QVariant>

class VGCoordinate;
class VGOutline;
class VGLandInformation;

class VGSupportPolyline : public MapAbstractItem
{
    Q_OBJECT
    Q_PROPERTY(QVariantList path READ path NOTIFY pathChanged)
public:
    explicit VGSupportPolyline(QObject *parent = 0);
    explicit VGSupportPolyline(const VGSupportPolyline &oth);

    void SetBegin(const QGeoCoordinate &co);
    void SetEnd(const QGeoCoordinate &co);
    void AddPoint(const QGeoCoordinate &co);
    QVariantList path()const;
    MapItemType ItemType() const;
    void SetValid(bool b);
    bool IsValide()const;
    bool operator==(const MapAbstractItem &oth)const;
    int CountPoint()const;
    void Clear();
protected:
signals :
    void   pathChanged(const QVariantList &path);
private:
    bool                    m_bValid;
    QVariantList            m_path;
};

#endif // __VG_SUPPORT_POLYLINE_H__
