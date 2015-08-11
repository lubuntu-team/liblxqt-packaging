/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXQt - a lightweight, Qt based, desktop toolset
 * http://razor-qt.org
 *
 * Copyright: 2010-2011 Razor team
 * Authors:
 *   Alexander Sokoloff <sokoloff.a@gmail.com>
 *   Petr Vanek <petr@scribus.info>
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

#include "lxqtsettings.h"
#include <QDebug>
#include <QEvent>
#include <QDir>
#include <QStringList>
#include <QMutex>
#include <QFileSystemWatcher>
#include <QSharedData>
#include <QTimerEvent>

#include <XdgIcon>
#include <XdgDirs>

using namespace LxQt;

class LxQt::SettingsPrivate
{
public:
    SettingsPrivate(Settings* parent):
        mChangeTimer(0),
        mParent(parent)
    {
    }

    QString localizedKey(const QString& key) const;

    QFileSystemWatcher mWatcher;
    int mChangeTimer;

private:
    Settings* mParent;
};


LxQtTheme* LxQtTheme::mInstance = 0;

class LxQt::LxQtThemeData: public QSharedData {
public:
    LxQtThemeData(): mValid(false) {}
    QString loadQss(const QString& qssFile) const;
    QString findTheme(const QString &themeName);

    QString mName;
    QString mPath;
    QString mPreviewImg;
    bool mValid;

};


class LxQt::GlobalSettingsPrivate
{
public:
    GlobalSettingsPrivate(GlobalSettings *parent):
        mParent(parent),
        mThemeUpdated(0ull)
    {

    }

    GlobalSettings *mParent;
    QString mIconTheme;
    QString mLxQtTheme;
    qlonglong mThemeUpdated;

};


/************************************************

 ************************************************/
Settings::Settings(const QString& module, QObject* parent) :
    QSettings("lxqt", module, parent),
    d_ptr(new SettingsPrivate(this))
{
    // HACK: we need to ensure that the user (~/.config/lxqt/<module>.conf)
    //       exists to have functional mWatcher
    if (!contains("__userfile__"))
    {
        setValue("__userfile__", true);
        sync();
    }
    d_ptr->mWatcher.addPath(this->fileName());
    connect(&(d_ptr->mWatcher), &QFileSystemWatcher::fileChanged, this, &Settings::_fileChanged);
}


/************************************************

 ************************************************/
Settings::Settings(const QString &fileName, QSettings::Format format, QObject *parent):
    QSettings(fileName, format, parent),
    d_ptr(new SettingsPrivate(this))
{
    // HACK: we need to ensure that the user (~/.config/lxqt/<module>.conf)
    //       exists to have functional mWatcher
    if (!contains("__userfile__"))
    {
        setValue("__userfile__", true);
        sync();
    }
    d_ptr->mWatcher.addPath(this->fileName());
    connect(&(d_ptr->mWatcher), &QFileSystemWatcher::fileChanged, this, &Settings::_fileChanged);
}


/************************************************

 ************************************************/
Settings::Settings(const QSettings* parentSettings, const QString& subGroup, QObject* parent):
    QSettings(parentSettings->organizationName(), parentSettings->applicationName(), parent),
    d_ptr(new SettingsPrivate(this))
{
    beginGroup(subGroup);
}


/************************************************

 ************************************************/
Settings::Settings(const QSettings& parentSettings, const QString& subGroup, QObject* parent):
    QSettings(parentSettings.organizationName(), parentSettings.applicationName(), parent),
    d_ptr(new SettingsPrivate(this))
{
    beginGroup(subGroup);
}


/************************************************

 ************************************************/
Settings::~Settings()
{
    // because in the Settings::Settings(const QString& module, QObject* parent)
    // constructor there is no beginGroup() called...
    if (!group().isEmpty())
        endGroup();

    delete d_ptr;
}

bool Settings::event(QEvent *event)
{
    if (event->type() == QEvent::UpdateRequest)
    {
        emit settingsChanged();
    }
    else if (event->type() == QEvent::Timer)
    {
        if(static_cast<QTimerEvent*>(event)->timerId() == d_ptr->mChangeTimer)
        {
            fileChanged(); // invoke the real fileChanged() handler.
            killTimer(d_ptr->mChangeTimer);
            d_ptr->mChangeTimer = 0;
        }
    }

    return QSettings::event(event);
}

void Settings::fileChanged()
{
    sync();
    emit settingsChanged();
}

void Settings::_fileChanged(QString path)
{
    // delay the change notification for 100 ms to avoid
    // unnecessary repeated loading of the same config file if
    // the file is changed for several times rapidly.
    if(d_ptr->mChangeTimer)
        killTimer(d_ptr->mChangeTimer);
    d_ptr->mChangeTimer = startTimer(100);

    // D*mn! yet another Qt 5.4 regression!!!
    // See the bug report: https://github.com/lxde/lxqt/issues/441
    // Since Qt 5.4, QSettings uses QSaveFile to save the config files.
    // https://github.com/qtproject/qtbase/commit/8d15068911d7c0ba05732e2796aaa7a90e34a6a1#diff-e691c0405f02f3478f4f50a27bdaecde
    // QSaveFile will save the content to a new temp file, and replace the old file later.
    // Hence the existing config file is not changed. Instead, it's deleted and then replaced.
    // This new behaviour unfortunately breaks QFileSystemWatcher.
    // After file deletion, we can no longer receive any new change notifications.
    // The most ridiculous thing is, QFileSystemWatcher does not provide a
    // way for us to know if a file is deleted. WT*?
    // Luckily, I found a workaround: If the file path no longer exists
    // in the watcher's files(), this file is deleted.
    if(!d_ptr->mWatcher.files().contains(path))
        d_ptr->mWatcher.addPath(path);
}


/************************************************

 ************************************************/
const GlobalSettings *Settings::globalSettings()
{
    static QMutex mutex;
    static GlobalSettings *instance = 0;
    if (!instance)
    {
        mutex.lock();

        if (!instance)
            instance = new GlobalSettings();

        mutex.unlock();
    }

    return instance;
}


/************************************************
 LC_MESSAGES value	Possible keys in order of matching
 lang_COUNTRY@MODIFIER	lang_COUNTRY@MODIFIER, lang_COUNTRY, lang@MODIFIER, lang,
                        default value
 lang_COUNTRY	        lang_COUNTRY, lang, default value
 lang@MODIFIER	        lang@MODIFIER, lang, default value
 lang	                lang, default value
 ************************************************/
QString SettingsPrivate::localizedKey(const QString& key) const
{

    QString lang = getenv("LC_MESSAGES");

    if (lang.isEmpty())
        lang = getenv("LC_ALL");

    if (lang.isEmpty())
         lang = getenv("LANG");


    QString modifier = lang.section('@', 1);
    if (!modifier.isEmpty())
        lang.truncate(lang.length() - modifier.length() - 1);

    QString encoding = lang.section('.', 1);
    if (!encoding.isEmpty())
        lang.truncate(lang.length() - encoding.length() - 1);


    QString country = lang.section('_', 1);
    if (!country.isEmpty())
        lang.truncate(lang.length() - country.length() - 1);



    //qDebug() << "LC_MESSAGES: " << getenv("LC_MESSAGES");
    //qDebug() << "Lang:" << lang;
    //qDebug() << "Country:" << country;
    //qDebug() << "Encoding:" << encoding;
    //qDebug() << "Modifier:" << modifier;

    if (!modifier.isEmpty() && !country.isEmpty())
    {
        QString k = QString("%1[%2_%3@%4]").arg(key, lang, country, modifier);
        //qDebug() << "\t try " << k << mParent->contains(k);
        if (mParent->contains(k))
            return k;
    }

    if (!country.isEmpty())
    {
        QString k = QString("%1[%2_%3]").arg(key, lang, country);
        //qDebug() << "\t try " << k  << mParent->contains(k);
        if (mParent->contains(k))
            return k;
    }

    if (!modifier.isEmpty())
    {
        QString k = QString("%1[%2@%3]").arg(key, lang, modifier);
        //qDebug() << "\t try " << k  << mParent->contains(k);
        if (mParent->contains(k))
            return k;
    }

    QString k = QString("%1[%2]").arg(key, lang);
    //qDebug() << "\t try " << k  << mParent->contains(k);
    if (mParent->contains(k))
        return k;


    //qDebug() << "\t try " << key  << mParent->contains(key);
    return key;
}

/************************************************

 ************************************************/
QVariant Settings::localizedValue(const QString& key, const QVariant& defaultValue) const
{
    Q_D(const Settings);
    return value(d->localizedKey(key), defaultValue);
}


/************************************************

 ************************************************/
void Settings::setLocalizedValue(const QString &key, const QVariant &value)
{
    Q_D(const Settings);
    setValue(d->localizedKey(key), value);
}


/************************************************

 ************************************************/
LxQtTheme::LxQtTheme():
    d(new LxQtThemeData)
{
}


/************************************************

 ************************************************/
LxQtTheme::LxQtTheme(const QString &path):
    d(new LxQtThemeData)
{
    if (path.isEmpty())
        return;

    QFileInfo fi(path);
    if (fi.isAbsolute())
    {
        d->mPath = path;
        d->mName = fi.fileName();
        d->mValid = fi.isDir();
    }
    else
    {
        d->mName = path;
        d->mPath = d->findTheme(path);
        d->mValid = !(d->mPath.isEmpty());
    }

    if (QDir(path).exists("preview.png"))
        d->mPreviewImg = path + "/preview.png";
}


/************************************************

 ************************************************/
QString LxQtThemeData::findTheme(const QString &themeName)
{
    if (themeName.isEmpty())
        return "";

    QStringList paths;
    paths << XdgDirs::dataHome(false);
    paths << XdgDirs::dataDirs();
    // TODO/FIXME: this is fallback path for standard CMAKE_INSTALL_PREFIX
    paths << "/usr/local/share";

    foreach(QString path, paths)
    {
        QDir dir(QString("%1/lxqt/themes/%2").arg(path, themeName));
        if (dir.isReadable())
            return dir.absolutePath();
    }

    return QString();
}


/************************************************

 ************************************************/
LxQtTheme::LxQtTheme(const LxQtTheme &other):
    d(other.d)
{
}


/************************************************

 ************************************************/
LxQtTheme::~LxQtTheme()
{
}


/************************************************

 ************************************************/
LxQtTheme& LxQtTheme::operator=(const LxQtTheme &other)
{
    d = other.d;
    return *this;
}


/************************************************

 ************************************************/
bool LxQtTheme::isValid() const
{
    return d->mValid;
}


/************************************************

 ************************************************/
QString LxQtTheme::name() const
{
    return d->mName;
}

/************************************************

 ************************************************/
QString LxQtTheme::path() const
{
    return d->mPath;
}


/************************************************

 ************************************************/
QString LxQtTheme::previewImage() const
{
    return d->mPreviewImg;
}


/************************************************

 ************************************************/
QString LxQtTheme::qss(const QString& module) const
{
    QString path = QString("%1/%2.qss").arg(d->mPath, module);

    QString styleSheet;
    if (!path.isEmpty())
        styleSheet = d->loadQss(path);
    else
        qWarning() << QString("QSS file %1 cannot be found").arg(path);

    // Single/double click ...........................
    Settings s("desktop");
    bool singleClick = s.value("icon-launch-mode", "singleclick").toString() == "singleclick";
    styleSheet += QString("QAbstractItemView {activate-on-singleclick : %1; }").arg(singleClick ? 1 : 0);

    return styleSheet;
}


/************************************************

 ************************************************/
QString LxQtThemeData::loadQss(const QString& qssFile) const
{
    QFile f(qssFile);
    if (! f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qWarning() << "Theme: Cannot open file for reading:" << qssFile;
        return QString();
    }

    QString qss = f.readAll();
    f.close();

    if (qss.isEmpty())
        return QString();

    // handle relative paths
    QString qssDir = QFileInfo(qssFile).canonicalPath();
    qss.replace(QRegExp("url.[ \\t\\s]*", Qt::CaseInsensitive, QRegExp::RegExp2), "url(" + qssDir + "/");

    return qss;
}


/************************************************

 ************************************************/
QString LxQtTheme::desktopBackground(int screen) const
{
    QString wallpaperCfgFileName = QString("%1/wallpaper.cfg").arg(d->mPath);

    if (wallpaperCfgFileName.isEmpty())
        return QString();

    QSettings s(wallpaperCfgFileName, QSettings::IniFormat);
    QString themeDir = QFileInfo(wallpaperCfgFileName).absolutePath();
    // There is something strange... If I remove next line the wallpapers array is not found...
    s.childKeys();
    s.beginReadArray("wallpapers");

    s.setArrayIndex(screen - 1);
    if (s.contains("file"))
        return QString("%1/%2").arg(themeDir, s.value("file").toString());

    s.setArrayIndex(0);
    if (s.contains("file"))
        return QString("%1/%2").arg(themeDir, s.value("file").toString());

    return QString();
}


/************************************************

 ************************************************/
const LxQtTheme &LxQtTheme::currentTheme()
{
    static LxQtTheme theme;
    QString name = Settings::globalSettings()->value("theme").toString();
    if (theme.name() != name)
    {
        theme = LxQtTheme(name);
    }
    return theme;
}


/************************************************

 ************************************************/
QList<LxQtTheme> LxQtTheme::allThemes()
{
    QList<LxQtTheme> ret;
    QSet<QString> processed;

    QStringList paths;
    paths << XdgDirs::dataHome(false);
    paths << XdgDirs::dataDirs();

    foreach(QString path, paths)
    {
        QDir dir(QString("%1/lxqt/themes").arg(path));
        QFileInfoList dirs = dir.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot);

        foreach(QFileInfo dir, dirs)
        {
            if (!processed.contains(dir.fileName()) &&
                 QDir(dir.absoluteFilePath()).exists("lxqt-panel.qss"))
            {
                processed << dir.fileName();
                ret << LxQtTheme(dir.absoluteFilePath());
            }

        }
    }

    return ret;
}


/************************************************

 ************************************************/
SettingsCache::SettingsCache(QSettings &settings) :
    mSettings(settings)
{
    loadFromSettings();
}


/************************************************

 ************************************************/
SettingsCache::SettingsCache(QSettings *settings) :
    mSettings(*settings)
{
    loadFromSettings();
}


/************************************************

 ************************************************/
void SettingsCache::loadFromSettings()
{
   foreach (QString key, mSettings.allKeys())
   {
       mCache.insert(key, mSettings.value(key));
   }
}


/************************************************

 ************************************************/
void SettingsCache::loadToSettings()
{
    QHash<QString, QVariant>::const_iterator i = mCache.constBegin();

    while(i != mCache.constEnd())
    {
        mSettings.setValue(i.key(), i.value());
        ++i;
    }

    mSettings.sync();
}


/************************************************

 ************************************************/
GlobalSettings::GlobalSettings():
    Settings("lxqt"),
    d_ptr(new GlobalSettingsPrivate(this))
{
    if (value("icon_theme").toString().isEmpty())
    {
        QStringList failback;
        failback << "oxygen";
        failback << "Tango";
        failback << "Prudence-icon";
        failback << "Humanity";
        failback << "elementary";
        failback << "gnome";


        QDir dir("/usr/share/icons/");
        foreach (QString s, failback)
        {
            if (dir.exists(s))
            {
                setValue("icon_theme", s);
                sync();
                break;
            }
        }
    }

    fileChanged();
}

GlobalSettings::~GlobalSettings()
{
    delete d_ptr;
}


/************************************************

 ************************************************/
void GlobalSettings::fileChanged()
{
    Q_D(GlobalSettings);
    sync();


    QString it = value("icon_theme").toString();
    if (d->mIconTheme != it)
    {
        d->mIconTheme = it;
        XdgIcon::setThemeName(it);
        emit iconThemeChanged();
    }

    QString rt = value("theme").toString();
    qlonglong themeUpdated = value("__theme_updated__").toLongLong();
    if ((d->mLxQtTheme != rt) || (d->mThemeUpdated != themeUpdated))
    {
        d->mLxQtTheme = rt;
        emit lxqtThemeChanged();
    }

    emit settingsChanged();
}

