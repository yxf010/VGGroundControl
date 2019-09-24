#include "VGImageProvider.h"
#include "VGIconEngine.h"

VGImageProvider::VGImageProvider(): QQuickImageProvider(QQuickImageProvider::Image)
{
}

QImage VGImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    QIcon::Mode md;
    QIcon ic = parseIcon(id, md);
    if (ic.isNull())
        return QImage();

    if (size)
        *size = requestedSize;
    return ic.pixmap(requestedSize, md).toImage();
}

QIcon VGImageProvider::parseIcon(const QString &id, QIcon::Mode &mode)
{
    int nCount = id.lastIndexOf("/");
    QString strUsr = nCount >= 0 ? id.mid(nCount + 1) : id;
    QStringList strLs = strUsr.split("%7C");
    if (strLs.size() == 1)
    {
        mode = QIcon::Normal;
        strUsr = strLs.at(0);
    }
    else if (strLs.size() == 2)
    {
        mode = (QIcon::Mode)strLs.at(1).toInt();
        strUsr = strLs.at(0);
    }
    else
    {
        return QIcon();
    }
    QMap<QString, QIcon>::Iterator itr = m_icons.find(strUsr);
    if (itr != m_icons.end())
        return itr.value();

    QIcon ic(new VGIconEngine(strUsr));
    m_icons[strUsr] = ic;
    return ic;
}

