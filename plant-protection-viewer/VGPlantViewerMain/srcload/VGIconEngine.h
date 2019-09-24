#ifndef __VGICONENGINE_H__
#define __VGICONENGINE_H__

#include <QIconEngine>
#include <QIcon>

class VGIconEngine : public QIconEngine
{
public:
    VGIconEngine(const QString &name);
    VGIconEngine(const VGIconEngine & oth);
    virtual void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state);
    virtual QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state);
    virtual QSize actualSize(const QSize &size, QIcon::Mode mode, QIcon::State state);
    virtual QString key() const;
    virtual QIconEngine *clone() const;
    virtual void virtual_hook(int id, void *data);
protected:
    void ensureLoaded();
    void serchFormat(const QString &name, const QString &fmt = "png");
private:
    QString m_name;
    QIcon m_icon;
    bool m_loaded;
};

#endif // IMAGEPROVIDER_H
