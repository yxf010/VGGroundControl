#ifndef AUDIOWORKER_H
#define AUDIOWORKER_H

#include <QTextToSpeech>

class AudioWorker : public QObject
{
    Q_OBJECT
    typedef QPair<QString, QStringList> AudioItem;
public:
    explicit AudioWorker(QObject *parent = nullptr);
    ~AudioWorker();

    void say(const QString &value, const QStringList &param);
    int countVoiceQue();
private slots :
    void onStateChanged(QTextToSpeech::State state);
    void onPrcsSayQue(const QString &value, const QStringList &param);
signals:
    void begingSay(const QString &value, const QStringList &param);
private:
    AudioItem *_getAudioItem(const QString &value);
    void _addAudioItem(const QString &value, const QStringList &param);
    void _speak();
    QString _genSpeakString(const QString &str, bool bParam=false);
private:
    QTextToSpeech       *m_tts;
    QList<AudioItem>    m_queAudioLoop;
    QStringList         m_queAudioMed;
};

#endif // AUDIOWORKER_H
