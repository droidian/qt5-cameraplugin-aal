/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QtTest/QtTest>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegExp>

#define private public
#include "storagemanager.h"

const QLatin1String testPath("/tmp/aalCameraStorageManagerUnitTestDirectory0192837465/");

class tst_StorageManager : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();
    void checkDirectory();
    void fileNameGenerator_data();
    void fileNameGenerator();

private:
    void removeTestDirectory();
};

void tst_StorageManager::initTestCase()
{
    removeTestDirectory();
}

void tst_StorageManager::cleanupTestCase()
{
    removeTestDirectory();
}

void tst_StorageManager::checkDirectory()
{
    StorageManager storage;

    QString path = testPath;
    bool ok = storage.checkDirectory(path);
    QCOMPARE(ok, true);
    QDir dir(path);
    ok = dir.exists();
    QCOMPARE(ok, true);
    dir.rmdir(path);

    QString file = "image007.jpg";
    ok = storage.checkDirectory(path+file);
    QCOMPARE(ok, true);
    dir.setPath(path);
    ok = dir.exists();
    QCOMPARE(ok, true);
    dir.rmdir(path);
}

void tst_StorageManager::fileNameGenerator_data()
{
    QTest::addColumn<QString>("extension");

    QTest::newRow("jpg") << "jpg";
    QTest::newRow("mp4") << "mp4";
}

void tst_StorageManager::fileNameGenerator()
{
    QFETCH(QString, extension);

    StorageManager storage;
    storage.m_directory = "/tmp";
    const QLatin1String photoBase("image");

    QString basePath = QString("/tmp/%1").arg(photoBase);
    QString date = QDate::currentDate().toString("yyyyMMdd");
    QRegExp pattern(QString("%1\\d{8}_\\d{9}\\.%2").arg(basePath).arg(extension));
    QString expectedPre = QString("%1%2").arg(basePath).arg(date);

    QString generated = storage.fileNameGenerator(photoBase, extension);
    QVERIFY(pattern.exactMatch(generated));

    QStringList parts = generated.split('_');
    QCOMPARE(parts.count(), 2);
    QString pre = parts[0];
    QCOMPARE(pre, expectedPre);
}

void tst_StorageManager::removeTestDirectory()
{
    QDir dir(testPath);
    QStringList fileNames = dir.entryList(QDir::Files);
    foreach (QString fileName, fileNames) {
        QFile::remove(testPath+fileName);
    }

    if (dir.exists())
        dir.rmdir(testPath);
}

QTEST_GUILESS_MAIN(tst_StorageManager);

#include "tst_storagemanager.moc"
