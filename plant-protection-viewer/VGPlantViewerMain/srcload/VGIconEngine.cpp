#include "VGIconEngine.h"
#include <QFile>
#include <QUrl>

VGIconEngine::VGIconEngine(const QString &name):QIconEngine()
, m_name(name), m_loaded(false)
{
}

VGIconEngine::VGIconEngine(const VGIconEngine &oth)
  : m_name(oth.m_name), m_loaded(oth.m_loaded), m_icon(oth.m_icon)
{
}

void VGIconEngine::paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state)
{
    ensureLoaded();
    m_icon.paint(painter, rect, Qt::AlignCenter, mode, state);
}

QPixmap VGIconEngine::pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    ensureLoaded();
    return m_icon.pixmap(size, mode, state);
}

QSize VGIconEngine::actualSize(const QSize &size, QIcon::Mode mode, QIcon::State state)
{
    ensureLoaded();
    return m_icon.actualSize(size, mode, state);
}

QString VGIconEngine::key() const
{
    return "VGIconEngine";
}

QIconEngine * VGIconEngine::clone() const
{
    return new VGIconEngine(*this);
}

void VGIconEngine::virtual_hook(int id, void *data)
{
    if (id == QIconEngine::AvailableSizesHook)
    {
        ensureLoaded();
        AvailableSizesArgument * a = (AvailableSizesArgument*)data;
        a->sizes = m_icon.availableSizes(a->mode, a->state);
        return;
    }
    else if (id == QIconEngine::IconNameHook)
    {
        QString * name = (QString *)data;
        *name = m_name;
        return;
    }

    return QIconEngine::virtual_hook(id, data);
}

void VGIconEngine::ensureLoaded()
{
    if (m_loaded)
        return;
    serchFormat(m_name, "png");
    serchFormat(m_name, "svg");

    m_loaded = true;
}

void VGIconEngine::serchFormat(const QString &name, const QString &fmt)
{
    QString path = QString(":/images/%1.%2").arg(name).arg(fmt);
    if (QFile::exists(path))
    {
        m_icon.addFile(path);
        path = QString(":/images/%1_%2.%3").arg(name).arg("gray").arg(fmt);
        if (QFile::exists(path))
            m_icon.addFile(path, QSize(), QIcon::Disabled);

        path = QString(":/images/%1_%2.%3").arg(name).arg("prs").arg(fmt);
        if (QFile::exists(path))
            m_icon.addFile(path, QSize(), QIcon::Selected);
    }
}
