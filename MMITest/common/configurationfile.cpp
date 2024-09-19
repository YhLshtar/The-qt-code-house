#include <QDebug>
#include "configurationfile.h"

ConfigurationFile::ConfigurationFile()
{

}

ConfigurationFile::~ConfigurationFile()
{
    if(settings != nullptr )
    {
       delete settings;
    }
}

void ConfigurationFile::set_file_path(QString file_path)
{
    config_file_path = file_path;
    if(settings != nullptr )
    {
        delete settings;
        settings = nullptr;
    }
    set_configuration();
}

void ConfigurationFile::set_configuration()
{
    QFileInfo fileInfo(config_file_path);
    if (!fileInfo.isFile())
    {
        QFile file(config_file_path);
        file.open(QIODevice::WriteOnly);
        file.close();
    }
    else
    {

    }

    settings = new QSettings(config_file_path, QSettings::IniFormat);
}

void ConfigurationFile::write(const QString label, const QString value)
{
    QStringList label_list = label.split("/");
    settings->beginGroup(label_list[0]);
    settings->setValue(label_list[1], value);
    settings->endGroup();
}

QString ConfigurationFile::read(const QString label)
{
    return settings->value(label).toString();
}

