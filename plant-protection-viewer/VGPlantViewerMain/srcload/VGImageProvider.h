#ifndef IMAGEPROVIDER_H
#define IMAGEPROVIDER_H

#include <QQuickImageProvider>
#include <QIcon>
#include <QMap>

class VGImageProvider : public QQuickImageProvider
{
public:
    VGImageProvider();
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize);
private:
    QIcon parseIcon(const QString &id, QIcon::Mode &mode);
private:
    QMap<QString, QIcon>    m_icons;
};

#endif // IMAGEPROVIDER_H
