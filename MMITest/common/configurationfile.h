#ifndef CONFIGURATIONFILE_H
#define CONFIGURATIONFILE_H

#include <QSettings>
#include <QFileInfo>
#include <QCoreApplication>

class ConfigurationFile
{
public:
    ConfigurationFile();
    virtual ~ConfigurationFile();

    QSettings *settings = nullptr;

    void set_file_path(QString file_path);
    void write(const QString label, const QString value);
    QString read(const QString label);

private:
    QString   config_file_path;

    void set_configuration();

};

#endif // CONFIGURATIONFILE_H
