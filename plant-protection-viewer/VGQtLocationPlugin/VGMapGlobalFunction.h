#ifndef VGMAPGLOBALFUNCTION_H
#define VGMAPGLOBALFUNCTION_H

#include <QPointF>
#include <QPoint>

//以下是根据百度地图JavaScript API破解得到 百度坐标<->墨卡托坐标 转换算法
double* Convertor(double x, double y, double* param);
//百度转墨卡托坐标
QPointF LatLng2Mercator(double lat, double lon);
//墨卡托坐标转百度
QPointF Mercator2LatLng(QPointF p);

//经纬度转百度瓦片，百度地图经纬度转瓦片算法（3－19级地图）
QPoint latLon2TileXYBd(double lon, double lat, int zoom);

//wsg瓦片转经纬度
double tilex2long(int x, int z);
double tiley2lat(int y, int z);

//google瓦片编号转换成百度瓦片编号
void transGTile2BTile(int gx, int gy, int zoom, int &bx, int &by);

#endif // VGMAPGLOBALFUNCTION_H
