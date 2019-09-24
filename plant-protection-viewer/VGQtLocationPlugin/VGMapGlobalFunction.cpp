#include "VGMapGlobalFunction.h"
#include <QtMath>


//以下是根据百度地图JavaScript API破解得到 百度坐标<->墨卡托坐标 转换算法
static double array1[] = { 75, 60, 45, 30, 15, 0 };
static double array3[] = { 12890594.86, 8362377.87, 5591021, 3481989.83, 1678043.12, 0 };
static double array2[6][10] = {{-0.0015702102444, 111320.7020616939, 1704480524535203, -10338987376042340, 26112667856603880, -35149669176653700, 26595700718403920, -10725012454188240, 1800819912950474, 82.5}
                         ,{0.0008277824516172526, 111320.7020463578, 647795574.6671607, -4082003173.641316, 10774905663.51142, -15171875531.51559, 12053065338.62167, -5124939663.577472, 913311935.9512032, 67.5}
                         ,{0.00337398766765, 111320.7020202162, 4481351.045890365, -23393751.19931662, 79682215.47186455, -115964993.2797253, 97236711.15602145, -43661946.33752821, 8477230.501135234, 52.5}
                         ,{0.00220636496208, 111320.7020209128, 51751.86112841131, 3796837.749470245, 992013.7397791013, -1221952.21711287, 1340652.697009075, -620943.6990984312, 144416.9293806241, 37.5}
                         ,{-0.0003441963504368392, 111320.7020576856, 278.2353980772752, 2485758.690035394, 6070.750963243378, 54821.18345352118, 9540.606633304236, -2710.55326746645, 1405.483844121726, 22.5}
                         ,{-0.0003218135878613132, 111320.7020701615, 0.00369383431289, 823725.6402795718, 0.46104986909093, 2351.343141331292, 1.58060784298199, 8.77738589078284, 0.37238884252424, 7.45}};

static double array4[6][10] = {{1.410526172116255e-8, 0.00000898305509648872, -1.9939833816331, 200.9824383106796, -187.2403703815547, 91.6087516669843, -23.38765649603339, 2.57121317296198, -0.03801003308653, 17337981.2}
                                     ,{-7.435856389565537e-9, 0.000008983055097726239, -0.78625201886289, 96.32687599759846, -1.85204757529826, -59.36935905485877, 47.40033549296737, -16.50741931063887, 2.28786674699375, 10260144.86}
                                     ,{-3.030883460898826e-8, 0.00000898305509983578, 0.30071316287616, 59.74293618442277, 7.357984074871, -25.38371002664745, 13.45380521110908, -3.29883767235584, 0.32710905363475, 6856817.37}
                                     ,{-1.981981304930552e-8, 0.000008983055099779535, 0.03278182852591, 40.31678527705744, 0.65659298677277, -4.44255534477492, 0.85341911805263, 0.12923347998204, -0.04625736007561, 4482777.06}
                                     ,{3.09191371068437e-9, 0.000008983055096812155, 0.00006995724062, 23.10934304144901, -0.00023663490511, -0.6321817810242, -0.00663494467273, 0.03430082397953, -0.00466043876332, 2555164.4}
                                     ,{2.890871144776878e-9, 0.000008983055095805407, -3.068298e-8, 7.47137025468032, -0.00000353937994, -0.02145144861037, -0.00001234426596, 0.00010322952773, -0.00000323890364, 826088.5}};


/*
 * 百度瓦片编号规则与谷歌一一对应关系；
 * 计算公式如下：

    谷歌瓦片行编号=[谷歌参照瓦片行编号+(百度行编号 – 百度参照瓦片行编号)] //向右，行为递增

    谷歌瓦片列编号=[谷歌参照瓦片列编号- (百度列编号 – 百度参照瓦片列编号)] //向上，列为递减

 * */
static QPoint g_arrayBaidu[19] = {QPoint(0, 0), QPoint(0, 0), QPoint(0, 0), QPoint(1, 0),
                               QPoint(3, 1), QPoint(6, 2), QPoint(12, 4), QPoint(24, 9),
                               QPoint(49, 18), QPoint(98, 36), QPoint(197, 73), QPoint(395, 147),
                               QPoint(790, 294), QPoint(1581, 589), QPoint(3163, 1178), QPoint(6327, 2356),
                               QPoint(12654, 4712), QPoint(25308, 9425), QPoint(50617, 18851)};

static QPoint g_arrayGoogle[19] = {QPoint(0, 0), QPoint(1, 0), QPoint(3, 1), QPoint(6, 2),
                               QPoint(13, 5), QPoint(26, 12), QPoint(52, 24), QPoint(105, 48),
                               QPoint(210, 97), QPoint(421, 194), QPoint(843, 387), QPoint(1685, 776),
                               QPoint(3372, 1551), QPoint(6744, 3013), QPoint(13489, 6207), QPoint(26978, 12415),
                               QPoint(53957, 24832), QPoint(107913, 49665), QPoint(215829, 99328)};

double* Convertor(double x, double y, double* param)
{
    double T = param[0] + param[1] * qAbs(x);
    double cC = qAbs(y) / param[9];
    double cF = param[2] + param[3] * cC + param[4] * cC * cC + param[5] * cC * cC * cC + param[6] * cC * cC * cC * cC + param[7] * cC * cC * cC * cC * cC + param[8] * cC * cC * cC * cC * cC * cC;
    T *= (x < 0 ? -1 : 1);
    cF *= (y < 0 ? -1 : 1);
    double* ret = new double[2];
    ret[0] = T;
    ret[1] = cF;
    return ret;
}

QPointF LatLng2Mercator(double lat, double lon)
{
    double* arr = NULL;
    double n_lat = lat > 74 ? 74 : lat;
    n_lat = n_lat < -74 ? -74 : n_lat;
    int len = sizeof(array1)/sizeof(double);
    for (int i = 0; i < len; i++)
    {
        if (lat >= array1[i])
        {
            arr = array2[i];
            break;
        }
    }

    if (arr == NULL)
    {
        for (int i = len - 1; i >= 0; i--)
        {
            if (lat <= -array1[i])
            {
                arr = array2[i];
                break;
            }
        }
    }

    double *res = Convertor(lon, lat, arr);
    QPointF pT;
    pT.setX((qreal)res[0]);
    pT.setY((qreal)res[1]);

    return pT;
}

//墨卡托坐标转百度
QPointF Mercator2LatLng(QPointF p)
{
    double* arr = NULL;
    QPointF* np = new QPointF(qAbs(p.x()), qAbs(p.y()));
    int len = sizeof(array3)/sizeof(double);
    for (int i = 0; i < len; i++)
    {
        if (np->y() >= array3[i])
        {
            arr = array4[i];
            break;
        }
   }
   double* res = Convertor(np->x(), np->y(), arr);
   QPointF pT;
   pT.setX((qreal)res[0]);
   pT.setY((qreal)res[1]);

   return pT;
}

QPoint latLon2TileXYBd(double lon, double lat, int zoom)
{
    QPoint ptTile;

    QPointF pt = LatLng2Mercator(lat, lon);

    int x = floor(pt.x() * pow(2, zoom - 18) / 256);
    int y = floor(pt.y() * pow(2, zoom - 18) / 256);

    ptTile.setX(x);
    ptTile.setY(y);

    return ptTile;
}

double tilex2long(int x, int z)
{
    double n = x / pow(2, z);
    return double(n * 360.0 - 180);
}

double tiley2lat(int y, int z)
{
    double n = M_PI - 2.0 * M_PI * y / pow(2.0, z);
    return double(180.0 / M_PI * qAtan(0.5 * (qExp(n) - qExp(-n))));
}

void transGTile2BTile(int gx, int gy, int zoom, int &bx, int &by)
{
    if(zoom < 0 || zoom > 19)
    {
        return;
    }

    bx = gx - g_arrayGoogle[zoom].x() + g_arrayBaidu[zoom].x();
    by = -(gy - g_arrayGoogle[zoom].y() - g_arrayBaidu[zoom].y());
}
