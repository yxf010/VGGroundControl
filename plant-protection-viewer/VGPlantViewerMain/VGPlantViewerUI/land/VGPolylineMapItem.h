#ifndef __VGPOLYLINEMAPITEM__
#define __VGPOLYLINEMAPITEM__

#include <QtLocation/private/qdeclarativegeomapitembase_p.h>
#include <QtLocation/private/qgeomapitemgeometry_p.h>

#include <QSGGeometryNode>
#include <QSGFlatColorMaterial>
#include <QGeoPath>

class VGMapPolylineNode;

class VGMapLineProperties : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qreal width READ width WRITE setWidth NOTIFY widthChanged)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
    Q_PROPERTY(int pxSeg READ pxSeg WRITE setPxSeg NOTIFY pxSegChanged)
public:
    explicit VGMapLineProperties(QObject *parent = 0);
    QColor color() const;
    void setColor(const QColor &color);
    qreal width() const;
    void setWidth(qreal width);
    int pxSeg() const;
    void setPxSeg(int px);
Q_SIGNALS:
    void widthChanged(qreal width);
    void colorChanged(const QColor &color);
    void pxSegChanged(int pxSeg);
private:
    qreal   m_width;
    QColor  m_color;
    int     m_pxSeg;
};

class VGPolylineGeometry : public QGeoMapItemGeometry
{
public:
    VGPolylineGeometry();

    void updateSourcePoints(const QGeoMap &map, const QList<QGeoCoordinate> &path, const QGeoCoordinate geoLeftBound);
    void updateScreenPoints(const QGeoMap &map, qreal strokeWidth, const VGMapLineProperties &line);
protected:
    QList<QList<QDoubleVector2D> > clipPath(const QGeoMap &map, const QList<QDoubleVector2D> &path, QDoubleVector2D &leftBoundWrapped);
    void clipPathToRect(const VGMapLineProperties &line, const QRectF &clipRect, QVector<qreal> &outPoints, QVector<QPainterPath::ElementType> &outTypes);
private:
    QVector<qreal> srcPoints_;
    QVector<QPainterPath::ElementType> srcPointTypes_;
};

class VGPolylineMapItem : public QDeclarativeGeoMapItemBase
{
    Q_OBJECT

    Q_PROPERTY(QJSValue path READ path WRITE setPath NOTIFY pathChanged)
    Q_PROPERTY(VGMapLineProperties *line READ line CONSTANT)
public:
    explicit VGPolylineMapItem(QQuickItem *parent = 0);
    ~VGPolylineMapItem();

    virtual void setMap(QDeclarativeGeoMap *quickMap, QGeoMap *map) Q_DECL_OVERRIDE;
    //from QuickItem
    virtual QSGNode *updateMapItemPaintNode(QSGNode *, UpdatePaintNodeData *) Q_DECL_OVERRIDE;

    Q_INVOKABLE int pathLength() const;
    Q_INVOKABLE void addCoordinate(const QGeoCoordinate &coordinate);
    Q_INVOKABLE void insertCoordinate(int index, const QGeoCoordinate &coordinate);
    Q_INVOKABLE void replaceCoordinate(int index, const QGeoCoordinate &coordinate);
    Q_INVOKABLE QGeoCoordinate coordinateAt(int index) const;
    Q_INVOKABLE bool containsCoordinate(const QGeoCoordinate &coordinate);
    Q_INVOKABLE void removeCoordinate(const QGeoCoordinate &coordinate);
    Q_INVOKABLE void removeCoordinate(int index);

    QJSValue path() const;
    virtual void setPath(const QJSValue &value);
    VGMapLineProperties *line();

    static QGeoCoordinate computeLeftBound(const QList<QGeoCoordinate> &path, QVector<double> &deltaXs, double &minX);
    static QGeoCoordinate updateLeftBound(const QList<QGeoCoordinate> &path, QVector<double> &deltaXs, double &minX, QGeoCoordinate currentLeftBound);
    static QGeoCoordinate getLeftBound(const QList<QGeoCoordinate> &path, QVector<double> &deltaXs, double &minX);
    static QGeoCoordinate getLeftBound(const QList<QGeoCoordinate> &path, QVector<double> &deltaXs, double &minX, QGeoCoordinate currentLeftBound);

    bool contains(const QPointF &point) const override;
    const QGeoShape &geoShape() const override;
    QGeoMap::ItemType itemType() const override;
Q_SIGNALS:
    void pathChanged();
protected:
    void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) Q_DECL_OVERRIDE;
    void setPathFromGeoList(const QList<QGeoCoordinate> &path);
    void updatePolish() Q_DECL_OVERRIDE;

    protected Q_SLOTS:
    void updateAfterLinePropertiesChanged();
    virtual void afterViewportChanged(const QGeoMapViewportChangeEvent &event) Q_DECL_OVERRIDE;

private:
    void pathPropertyChanged();

    VGMapLineProperties line_;
    QGeoPath geopath_;
    QGeoCoordinate geoLeftBound_;
    QColor color_;
    bool dirtyMaterial_;
    VGPolylineGeometry geometry_;
    bool updatingGeometry_;
    // for the left bound calculation
    QVector<double> deltaXs_; // longitude deltas from path_[0]
    double minX_;             // minimum value inside deltaXs_
};

//////////////////////////////////////////////////////////////////////

class VGMapPolylineNode : public QSGGeometryNode
{

public:
    VGMapPolylineNode();
    ~VGMapPolylineNode();

    void update(const QColor &fillColor, const QGeoMapItemGeometry *shape);
    bool isSubtreeBlocked() const;

private:
    QSGFlatColorMaterial fill_material_;
    QSGGeometry geometry_;
    bool blocked_;
};

#endif //#ifndef __QDECLARATIVEPOLYLINEMAPITEM__
