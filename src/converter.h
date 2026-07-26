#ifndef CONVERTER_H
#define CONVERTER_H

#include <QObject>
#include <QUrl>

class Converter : public QObject{
    Q_OBJECT
public:
    explicit Converter (QObject* parent = 0) : QObject(parent) {}
    Q_INVOKABLE void convert(const QUrl& inUrl, const QUrl& outUrl);
    Q_INVOKABLE QString getResult() const { return m_result; }
signals:
    void resultChanged(QString newValue);
private:
    QString m_result;
};

#endif // CONVERTER_H