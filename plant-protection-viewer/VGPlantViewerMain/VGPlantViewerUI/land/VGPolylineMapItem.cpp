#include "VGPolylineMapItem.h"
#include <QtLocation/private/qgeocameracapabilities_p.h>
#include <QtPositioning/private/qlocationutils_p.h>
#include <QtLocation/private/error_messages_p.h>
#include <QtLocation/private/locationvaluetypehelper_p.h>
#include <QtPositioning/private/qdoublevector2d_p.h>

#include <QtCore/QScopedValueRollback>
#include <QtQml/QQmlInfo>
#include <QtQml/private/qqmlengine_p.h>
#include <QPainter>
#include <QPainterPath>
#include <QPainterPathStroker>
#include <qnumeric.h>

#include <QtGui/private/qvectorpath_p.h>
#include <QtGui/private/qtriangulatingstroker_p.h>
#include <QtGui/private/qtriangulator_p.h>
#include "RouteStruct.h"

////////////////////////////////////////////////////////////////////////////
/* Polyline clip */
enum ClipPointType {
    InsidePoint = 0x00,
    LeftPoint = 0x01,
    RightPoint = 0x02,
    BottomPoint = 0x04,
    TopPoint = 0x08
};

static inline int clipPointType(qreal x, qreal y, const QRectF &rect)
{
    int type = InsidePoint;
    if (x < rect.left())
        type |= LeftPoint;
    else if (x > rect.right())
        type |= RightPoint;
    if (y < rect.top())
        type |= TopPoint;
    else if (y > rect.bottom())
        type |= BottomPoint;
    return type;
}

static int genDashLine(qreal x0, qreal y0, qreal x1, qreal y1, 
    const VGMapLineProperties &line,
    QVector<qreal> &outPoints,
    QVector<QPainterPath::ElementType> &outTypes )
{
    int i = 1;
    VGPoint pStart(x0, y0);
    VGVertex vx(pStart, VGPoint(x1, y1));
    qreal len = vx.UnitLength() / line.pxSeg();
    for (; i < len; ++i)
    {
        VGPoint p = pStart + vx *(i / len);
        outTypes << (i % 2 ? QPainterPath::LineToElement : QPainterPath::MoveToElement);
        outPoints << p.X() << p.Y();
    }
    return i;
}

static void clipSegmentToRect(qreal x0, qreal y0, qreal x1, qreal y1,
    const QRectF &clipRect,
    const VGMapLineProperties &line,
    QVector<qreal> &outPoints,
    QVector<QPainterPath::ElementType> &outTypes)
{
    int type0 = clipPointType(x0, y0, clipRect);
    int type1 = clipPointType(x1, y1, clipRect);
    bool accept = false;

    while (true)
    {
        if (!(type0 | type1)) {
            accept = true;
            break;
        }
        else if (type0 & type1) {
            break;
        }
        else {
            qreal x = 0.0;
            qreal y = 0.0;
            int outsideType = type0 ? type0 : type1;

            if (outsideType & BottomPoint) {
                x = x0 + (x1 - x0) * (clipRect.bottom() - y0) / (y1 - y0);
                y = clipRect.bottom() - 0.1;
            }
            else if (outsideType & TopPoint) {
                x = x0 + (x1 - x0) * (clipRect.top() - y0) / (y1 - y0);
                y = clipRect.top() + 0.1;
            }
            else if (outsideType & RightPoint) {
                y = y0 + (y1 - y0) * (clipRect.right() - x0) / (x1 - x0);
                x = clipRect.right() - 0.1;
            }
            else if (outsideType & LeftPoint) {
                y = y0 + (y1 - y0) * (clipRect.left() - x0) / (x1 - x0);
                x = clipRect.left() + 0.1;
            }

            if (outsideType == type0) {
                x0 = x;
                y0 = y;
                type0 = clipPointType(x0, y0, clipRect);
            }
            else {
                x1 = x;
                y1 = y;
                type1 = clipPointType(x1, y1, clipRect);
            }
        }
    }

    if (accept)
    {
        int i = 1;
        qreal lastX, lastY;
        if (outPoints.size() >= 2)
        {
            lastX = outPoints.at(outPoints.size() - 2);
            lastY = outPoints.at(outPoints.size() - 1);
            if (!qFuzzyCompare(lastY, y0) || !qFuzzyCompare(lastX, x0))
            {
                outTypes << QPainterPath::MoveToElement;
                outPoints << x0 << y0;
            }
        }
        else {
            outTypes << QPainterPath::MoveToElement;
            outPoints << x0 << y0;
            lastX = x0;
            lastY = y0;
        }

        if (line.pxSeg() > 1)
            i = genDashLine(lastX, lastY, x1, y1, line, outPoints, outTypes);
        outTypes << (i % 2 ? QPainterPath::LineToElement : QPainterPath::MoveToElement);
        outPoints << x1 << y1;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
//VGMapLineProperties
///////////////////////////////////////////////////////////////////////////////////////////////////
VGMapLineProperties::VGMapLineProperties(QObject *parent) :
    QObject(parent), m_width(1.0), m_color(Qt::black)
    , m_pxSeg(0)
{
}

QColor VGMapLineProperties::color() const
{
    return m_color;
}

void VGMapLineProperties::setColor(const QColor &color)
{
    if (m_color == color)
        return;

    m_color = color;
    emit colorChanged(color);
}

qreal VGMapLineProperties::width() const
{
    return m_width;
}

void VGMapLineProperties::setWidth(qreal width)
{
    if (m_width == width)
        return;

    m_width = width;
    emit widthChanged(m_width);
}

int VGMapLineProperties::pxSeg() const
{
    return m_pxSeg;
}

void VGMapLineProperties::setPxSeg(int px)
{
    if (m_pxSeg == px)
        return;

    m_pxSeg = px;
    emit pxSegChanged(px);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//VGPolylineGeometry
///////////////////////////////////////////////////////////////////////////////////////////////////
VGPolylineGeometry::VGPolylineGeometry()
{
}

void VGPolylineGeometry::updateSourcePoints(const QGeoMap &map,
    const QList<QGeoCoordinate> &path,
    const QGeoCoordinate geoLeftBound)
{
    bool foundValid = false;
    double minX = -1.0;
    double minY = -1.0;
    double maxX = -1.0;
    double maxY = -1.0;

    if (!sourceDirty_)
        return;

    geoLeftBound_ = geoLeftBound;

    // clear the old data and reserve enough memory
    srcPoints_.clear();
    srcPoints_.reserve(path.size() * 2);
    srcPointTypes_.clear();
    srcPointTypes_.reserve(path.size());

    QDoubleVector2D origin, lastPoint, lastAddedPoint;

    const QGeoProjectionWebMercator &p = static_cast<const QGeoProjectionWebMercator&>(map.geoProjection());
    const double mapWidthHalf = map.mapWidth() / 2.0;
    double unwrapBelowX = 0;
    if (preserveGeometry_)
        unwrapBelowX = p.coordinateToItemPosition(geoLeftBound_, false).x();

    for (int i = 0; i < path.size(); ++i) {
        const QGeoCoordinate &coord = path.at(i);

        if (!coord.isValid())
            continue;

        QDoubleVector2D point = p.coordinateToItemPosition(coord, false);

        // We can get NaN if the map isn't set up correctly, or the projection
        // is faulty -- probably best thing to do is abort
        if (!qIsFinite(point.x()) || !qIsFinite(point.y()))
            return;

        bool isPointLessThanUnwrapBelowX = (point.x() < unwrapBelowX);
        bool isCoordNotLeftBound = !qFuzzyCompare(geoLeftBound_.longitude(), coord.longitude());
        bool isPointNotUnwrapBelowX = !qFuzzyCompare(point.x(), unwrapBelowX);
        bool isPointNotMapWidthHalf = !qFuzzyCompare(mapWidthHalf, point.x());

        // unwrap x to preserve geometry if moved to border of map
        if (preserveGeometry_ && isPointLessThanUnwrapBelowX
            && isCoordNotLeftBound
            && isPointNotUnwrapBelowX
            && isPointNotMapWidthHalf) {
            double distance = geoDistanceToScreenWidth(map, geoLeftBound_, coord);
            point.setX(unwrapBelowX + distance);
        }

        if (!foundValid) {
            foundValid = true;
            srcOrigin_ = coord;  // TODO: Make this consistent with the left bound
            origin = point;
            point = QDoubleVector2D(0, 0);

            minX = point.x();
            maxX = minX;
            minY = point.y();
            maxY = minY;

            srcPoints_ << point.x() << point.y();
            srcPointTypes_ << QPainterPath::MoveToElement;
            lastAddedPoint = point;
        }
        else {
            point -= origin;

            minX = qMin(point.x(), minX);
            minY = qMin(point.y(), minY);
            maxX = qMax(point.x(), maxX);
            maxY = qMax(point.y(), maxY);

            if ((point - lastAddedPoint).manhattanLength() > 3 ||
                i == path.size() - 1) {
                srcPoints_ << point.x() << point.y();
                srcPointTypes_ << QPainterPath::LineToElement;
                lastAddedPoint = point;
            }
        }

        lastPoint = point;
    }

    sourceBounds_ = QRectF(QPointF(minX, minY), QPointF(maxX, maxY));
}

void VGPolylineGeometry::updateScreenPoints(const QGeoMap &map, qreal strokeWidth, const VGMapLineProperties &line)
{
    if (!screenDirty_)
        return;

    const QGeoProjectionWebMercator &p = static_cast<const QGeoProjectionWebMercator&>(map.geoProjection());
    QPointF origin = p.coordinateToItemPosition(srcOrigin_, false).toPointF();

    if (!qIsFinite(origin.x()) || !qIsFinite(origin.y())) {
        clear();
        return;
    }

    // Create the viewport rect in the same coordinate system
    // as the actual points
    QRectF viewport(0, 0, map.mapWidth(), map.mapHeight());
    viewport.adjust(-strokeWidth, -strokeWidth, strokeWidth, strokeWidth);
    viewport.translate(-1 * origin);

    // Perform clipping to the viewport limits
    QVector<qreal> points;
    QVector<QPainterPath::ElementType> types;

    if (clipToViewport_)
    {
        clipPathToRect(line, viewport, points, types);
    }
    else
    {
        points = srcPoints_;
        types = srcPointTypes_;
    }

    QVectorPath vp(points.data(), types.size(), types.data());
    QTriangulatingStroker ts;
    ts.process(vp, QPen(Qt::black, strokeWidth), viewport, QPainter::Qt4CompatiblePainting);

    clear();

    // Nothing is on the screen
    if (ts.vertexCount() == 0)
        return;

    // QTriangulatingStroker#vertexCount is actually the length of the array,
    // not the number of vertices
    screenVertices_.reserve(ts.vertexCount());

    QRectF bb;
    QPointF pt;
    const float *vs = ts.vertices();
    for (int i = 0; i < (ts.vertexCount() / 2 * 2); i += 2)
    {
        pt = QPointF(vs[i], vs[i + 1]);
        screenVertices_ << pt;

        if (!qIsFinite(pt.x()) || !qIsFinite(pt.y()))
            break;

        if (!bb.contains(pt)) {
            if (pt.x() < bb.left())
                bb.setLeft(pt.x());

            if (pt.x() > bb.right())
                bb.setRight(pt.x());

            if (pt.y() < bb.top())
                bb.setTop(pt.y());

            if (pt.y() > bb.bottom())
                bb.setBottom(pt.y());
        }
    }

    screenBounds_ = bb;
    this->translate(-1 * sourceBounds_.topLeft());
}

void VGPolylineGeometry::clipPathToRect(const VGMapLineProperties &line, const QRectF &clipRect, QVector<qreal> &outPoints, QVector<QPainterPath::ElementType> &outTypes)
{
    outPoints.clear();
    outPoints.reserve(srcPoints_.size());
    outTypes.clear();
    outTypes.reserve(srcPointTypes_.size());

    qreal lastX, lastY;
    for (int i = 0; i < srcPointTypes_.size(); ++i) {
        if (i > 0 && srcPointTypes_[i] != QPainterPath::MoveToElement) {
            qreal x = srcPoints_[i * 2], y = srcPoints_[i * 2 + 1];
            clipSegmentToRect(lastX, lastY, x, y, clipRect, line, outPoints, outTypes);
        }

        lastX = srcPoints_[i * 2];
        lastY = srcPoints_[i * 2 + 1];
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////
//VGPolylineMapItem
///////////////////////////////////////////////////////////////////////////////////////////////////
VGPolylineMapItem::VGPolylineMapItem(QQuickItem *parent)
    : QDeclarativeGeoMapItemBase(parent), dirtyMaterial_(true), updatingGeometry_(false)
{
    setFlag(ItemHasContents, true);
    QObject::connect(&line_, &VGMapLineProperties::colorChanged, this, &VGPolylineMapItem::updateAfterLinePropertiesChanged);
    QObject::connect(&line_, &VGMapLineProperties::widthChanged, this, &VGPolylineMapItem::updateAfterLinePropertiesChanged);
}

VGPolylineMapItem::~VGPolylineMapItem()
{
}

void VGPolylineMapItem::updateAfterLinePropertiesChanged()
{
    // mark dirty just in case we're a width change
    geometry_.markSourceDirty();
    polishAndUpdate();
}

void VGPolylineMapItem::setMap(QDeclarativeGeoMap *quickMap, QGeoMap *map)
{
    QDeclarativeGeoMapItemBase::setMap(quickMap, map);
    if (map) {
        geometry_.markSourceDirty();
        polishAndUpdate();
    }
}

QJSValue VGPolylineMapItem::path() const
{
    QQmlContext *context = QQmlEngine::contextForObject(this);
    if (!context)
        return QJSValue();
    QQmlEngine *engine = context->engine();
    QV4::ExecutionEngine *v4 = QQmlEnginePrivate::getV4Engine(engine);

    QV4::Scope scope(v4);
    QV4::Scoped<QV4::ArrayObject> pathArray(scope, v4->newArrayObject(geopath_.length()));
    for (int i = 0; i < geopath_.length(); ++i) {
        const QGeoCoordinate &c = geopath_.coordinateAt(i);

        QV4::ScopedValue cv(scope, v4->fromVariant(QVariant::fromValue(c)));
        pathArray->putIndexed(i, cv);
    }

    return QJSValue(v4, pathArray.asReturnedValue());
}

void VGPolylineMapItem::setPath(const QJSValue &value)
{
    if (!value.isArray())
        return;

    QList<QGeoCoordinate> pathList;
    quint32 length = value.property(QStringLiteral("length")).toUInt();
    for (quint32 i = 0; i < length; ++i) {
        bool ok;
        QGeoCoordinate c = parseCoordinate(value.property(i), &ok);

        if (!ok || !c.isValid()) {
            qmlInfo(this) << "Unsupported path type";
            return;
        }

        pathList.append(c);
    }

    setPathFromGeoList(pathList);
}

void VGPolylineMapItem::setPathFromGeoList(const QList<QGeoCoordinate> &path)
{
    if (geopath_.path() == path)
        return;

    geopath_.setPath(path);
    geoLeftBound_ = getLeftBound(path, deltaXs_, minX_);
    geometry_.setPreserveGeometry(true, geoLeftBound_);
    geometry_.markSourceDirty();
    polishAndUpdate();
    emit pathChanged();
}

int VGPolylineMapItem::pathLength() const
{
    return geopath_.size();
}

void VGPolylineMapItem::addCoordinate(const QGeoCoordinate &coordinate)
{
    geopath_.addCoordinate(coordinate);
    geoLeftBound_ = getLeftBound(geopath_.path(), deltaXs_, minX_, geoLeftBound_);
    geometry_.setPreserveGeometry(true, geoLeftBound_);
    geometry_.markSourceDirty();
    polishAndUpdate();
    emit pathChanged();
}

void VGPolylineMapItem::insertCoordinate(int index, const QGeoCoordinate &coordinate)
{
    if (index < 0 || index > geopath_.size())
        return;

    geopath_.insertCoordinate(index, coordinate);
    geoLeftBound_ = getLeftBound(geopath_.path(), deltaXs_, minX_);
    geometry_.setPreserveGeometry(true, geoLeftBound_);
    geometry_.markSourceDirty();
    polishAndUpdate();
    emit pathChanged();
}

void VGPolylineMapItem::replaceCoordinate(int index, const QGeoCoordinate &coordinate)
{
    if (index < 0 || index >= geopath_.size())
        return;

    geopath_.replaceCoordinate(index, coordinate);
    geoLeftBound_ = getLeftBound(geopath_.path(), deltaXs_, minX_);
    geometry_.setPreserveGeometry(true, geoLeftBound_);
    geometry_.markSourceDirty();
    polishAndUpdate();
    emit pathChanged();
}

QGeoCoordinate VGPolylineMapItem::coordinateAt(int index) const
{
    if (index < 0 || index >= geopath_.size())
        return QGeoCoordinate();

    return geopath_.coordinateAt(index);
}

bool VGPolylineMapItem::containsCoordinate(const QGeoCoordinate &coordinate)
{
    return geopath_.containsCoordinate(coordinate);
}

void VGPolylineMapItem::removeCoordinate(const QGeoCoordinate &coordinate)
{
    geopath_.removeCoordinate(coordinate);
}

void VGPolylineMapItem::removeCoordinate(int index)
{
    if (index < 0 || index >= geopath_.path().size())
        return;

    geopath_.removeCoordinate(index);
    geoLeftBound_ = getLeftBound(geopath_.path(), deltaXs_, minX_);
    geometry_.setPreserveGeometry(true, geoLeftBound_);
    geometry_.markSourceDirty();
    polishAndUpdate();
    emit pathChanged();
}

VGMapLineProperties *VGPolylineMapItem::line()
{
    return &line_;
}

QGeoCoordinate VGPolylineMapItem::computeLeftBound(const QList<QGeoCoordinate> &path,
    QVector<double> &deltaXs,
    double &minX)
{
    if (path.isEmpty()) {
        minX = qInf();
        return QGeoCoordinate();
    }
    deltaXs.resize(path.size());

    deltaXs[0] = minX = 0.0;
    int minId = 0;

    for (int i = 1; i < path.size(); i++) {
        const QGeoCoordinate &geoFrom = path.at(i - 1);
        const QGeoCoordinate &geoTo = path.at(i);
        double longiFrom = geoFrom.longitude();
        double longiTo = geoTo.longitude();
        double deltaLongi = longiTo - longiFrom;
        if (qAbs(deltaLongi) > 180.0) {
            if (longiTo > 0.0)
                longiTo -= 360.0;
            else
                longiTo += 360.0;
            deltaLongi = longiTo - longiFrom;
        }
        deltaXs[i] = deltaXs[i - 1] + deltaLongi;
        if (deltaXs[i] < minX) {
            minX = deltaXs[i];
            minId = i;
        }
    }
    return path.at(minId);
}

QGeoCoordinate VGPolylineMapItem::updateLeftBound(const QList<QGeoCoordinate> &path,
    QVector<double> &deltaXs,
    double &minX,
    QGeoCoordinate currentLeftBound)
{
    if (path.isEmpty()) {
        deltaXs.clear();
        minX = qInf();
        return QGeoCoordinate();
    }
    else if (path.size() == 1) {
        deltaXs.resize(1);
        deltaXs[0] = minX = 0.0;
        return path.last();
    }
    else if (path.size() != deltaXs.size() + 1) {  // something went wrong. This case should not happen
        return computeLeftBound(path, deltaXs, minX);
    }

    const QGeoCoordinate &geoFrom = path.at(path.size() - 2);
    const QGeoCoordinate &geoTo = path.last();
    double longiFrom = geoFrom.longitude();
    double longiTo = geoTo.longitude();
    double deltaLongi = longiTo - longiFrom;
    if (qAbs(deltaLongi) > 180.0) {
        if (longiTo > 0.0)
            longiTo -= 360.0;
        else
            longiTo += 360.0;
        deltaLongi = longiTo - longiFrom;
    }

    deltaXs.push_back(deltaXs.last() + deltaLongi);
    if (deltaXs.last() < minX) {
        minX = deltaXs.last();
        return path.last();
    }
    else {
        return currentLeftBound;
    }
}

QGeoCoordinate VGPolylineMapItem::getLeftBound(const QList<QGeoCoordinate> &path,
    QVector<double> &deltaXs,
    double &minX)
{
    return VGPolylineMapItem::computeLeftBound(path, deltaXs, minX);
}

// Optimizing the common addCoordinate() path
QGeoCoordinate VGPolylineMapItem::getLeftBound(const QList<QGeoCoordinate> &path,
    QVector<double> &deltaXs,
    double &minX,
    QGeoCoordinate currentLeftBound)
{
    return VGPolylineMapItem::updateLeftBound(path, deltaXs, minX, currentLeftBound);
}

const QGeoShape &VGPolylineMapItem::geoShape(void) const
{
    return geopath_;
}

QGeoMap::ItemType VGPolylineMapItem::itemType() const
{
    return QGeoMap::MapPolyline;
}

void VGPolylineMapItem::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    if (updatingGeometry_ || newGeometry.topLeft() == oldGeometry.topLeft()) {
        QDeclarativeGeoMapItemBase::geometryChanged(newGeometry, oldGeometry);
        return;
    }

    QDoubleVector2D newPoint = QDoubleVector2D(x(), y()) + QDoubleVector2D(geometry_.firstPointOffset());
    const QGeoProjectionWebMercator &p = static_cast<const QGeoProjectionWebMercator&>(map()->geoProjection());
    QGeoCoordinate newCoordinate = p.itemPositionToCoordinate(newPoint, false);
    const QList<QGeoCoordinate> &path = geopath_.path();
    if (newCoordinate.isValid()) {
        double firstLongitude = path.at(0).longitude();
        double firstLatitude = path.at(0).latitude();
        double minMaxLatitude = firstLatitude;
        // prevent dragging over valid min and max latitudes
        for (int i = 0; i < path.count(); ++i) {
            double newLatitude = path.at(i).latitude()
                + newCoordinate.latitude() - firstLatitude;
            if (!QLocationUtils::isValidLat(newLatitude)) {
                if (qAbs(newLatitude) > qAbs(minMaxLatitude)) {
                    minMaxLatitude = newLatitude;
                }
            }
        }
        // calculate offset needed to re-position the item within map border
        double offsetLatitude = minMaxLatitude - QLocationUtils::clipLat(minMaxLatitude);
        for (int i = 0; i < path.count(); ++i) {
            QGeoCoordinate coord = path.at(i);
            // handle dateline crossing
            coord.setLongitude(QLocationUtils::wrapLong(coord.longitude()
                + newCoordinate.longitude() - firstLongitude));
            coord.setLatitude(coord.latitude()
                + newCoordinate.latitude() - firstLatitude - offsetLatitude);
            geopath_.replaceCoordinate(i, coord);
        }

        geoLeftBound_.setLongitude(QLocationUtils::wrapLong(geoLeftBound_.longitude()
            + newCoordinate.longitude() - firstLongitude));
        geometry_.setPreserveGeometry(true, geoLeftBound_);
        geometry_.markSourceDirty();
        polishAndUpdate();
        emit pathChanged();
    }

    // Not calling QDeclarativeGeoMapItemBase::geometryChanged() as it will be called from a nested
    // call to this function.
}

void VGPolylineMapItem::afterViewportChanged(const QGeoMapViewportChangeEvent &event)
{
    if (event.mapSize.width() <= 0 || event.mapSize.height() <= 0)
        return;

    // if the scene is tilted, we must regenerate our geometry every frame
    if (map()->cameraCapabilities().supportsTilting()
        && (event.cameraData.tilt() > 0.1
            || event.cameraData.tilt() < -0.1)) {
        geometry_.markSourceDirty();
    }

    // if the scene is rolled, we must regen too
    if (map()->cameraCapabilities().supportsRolling()
        && (event.cameraData.roll() > 0.1
            || event.cameraData.roll() < -0.1)) {
        geometry_.markSourceDirty();
    }

    // otherwise, only regen on rotate, resize and zoom
    if (event.bearingChanged || event.mapSizeChanged || event.zoomLevelChanged) {
        geometry_.markSourceDirty();
    }
    geometry_.setPreserveGeometry(true, geometry_.geoLeftBound());
    geometry_.markScreenDirty();
    polishAndUpdate();
}

void VGPolylineMapItem::updatePolish()
{
    const QList<QGeoCoordinate> &path = geopath_.path();
    if (!map() || path.count() == 0)
        return;

    QScopedValueRollback<bool> rollback(updatingGeometry_);
    updatingGeometry_ = true;

    geometry_.updateSourcePoints(*map(), path, geoLeftBound_);
    geometry_.updateScreenPoints(*map(), line_.width(), line_);

    setWidth(geometry_.sourceBoundingBox().width());
    setHeight(geometry_.sourceBoundingBox().height());

    setPositionOnMap(path.at(0), -1 * geometry_.sourceBoundingBox().topLeft());
}

QSGNode *VGPolylineMapItem::updateMapItemPaintNode(QSGNode *oldNode, UpdatePaintNodeData *data)
{
    Q_UNUSED(data);

    VGMapPolylineNode *node = static_cast<VGMapPolylineNode *>(oldNode);

    if (!node) {
        node = new VGMapPolylineNode();
    }

    //TODO: update only material
    if (geometry_.isScreenDirty() || dirtyMaterial_ || !oldNode) {
        node->update(line_.color(), &geometry_);
        geometry_.setPreserveGeometry(false);
        geometry_.markClean();
        dirtyMaterial_ = false;
    }
    return node;
}

bool VGPolylineMapItem::contains(const QPointF &point) const
{
    QVector<QPointF> vertices = geometry_.vertices();
    QPolygonF tri;
    for (int i = 0; i < vertices.size(); ++i) {
        tri << vertices[i];
        if (tri.size() == 3) {
            if (tri.containsPoint(point, Qt::OddEvenFill))
                return true;
            tri.remove(0);
        }
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//VGMapPolylineNode
///////////////////////////////////////////////////////////////////////////////////////////////////
VGMapPolylineNode::VGMapPolylineNode() :
    geometry_(QSGGeometry::defaultAttributes_Point2D(), 0),
    blocked_(true)
{
    geometry_.setDrawingMode(GL_TRIANGLE_STRIP);
    QSGGeometryNode::setMaterial(&fill_material_);
    QSGGeometryNode::setGeometry(&geometry_);
}

VGMapPolylineNode::~VGMapPolylineNode()
{
}

bool VGMapPolylineNode::isSubtreeBlocked() const
{
    return blocked_;
}

void VGMapPolylineNode::update(const QColor &fillColor,
    const QGeoMapItemGeometry *shape)
{
    if (shape->size() == 0) {
        blocked_ = true;
        return;
    }
    else {
        blocked_ = false;
    }

    QSGGeometry *fill = QSGGeometryNode::geometry();
    shape->allocateAndFill(fill);
    markDirty(DirtyGeometry);

    if (fillColor != fill_material_.color()) {
        fill_material_.setColor(fillColor);
        setMaterial(&fill_material_);
        markDirty(DirtyMaterial);
    }
}