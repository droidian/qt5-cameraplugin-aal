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
    void nextFileName();
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

void tst_StorageManager::nextFileName()
{
    StorageManager storage;

    const QLatin1String photoBase("image");
    const QLatin1String videoBase("video");

    QString fileName = storage.nextPhotoFileName(testPath);
    QString compareFileName = storage.fileNameGenerator(1, photoBase,"jpg");
    QCOMPARE(fileName, compareFileName);

    QDir dir;
    dir.mkpath(testPath);
    QFile file(compareFileName);
    file.open(QIODevice::ReadWrite);
    file.close();
    fileName = storage.nextPhotoFileName(testPath);
    compareFileName = storage.fileNameGenerator(2, photoBase, "jpg");
    QCOMPARE(fileName, compareFileName);


    fileName = storage.nextVideoFileName(testPath);
    compareFileName = storage.fileNameGenerator(1, videoBase, "mp4");
    QCOMPARE(fileName, compareFileName);

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
    QTest::addColumn<int>("index");
    QTest::addColumn<QString>("idxString");
    QTest::addColumn<QString>("extension");

    QTest::newRow("1") << 1 << "0001" << "jpg";
    QTest::newRow("12") << 12 << "0012" << "mp4";
    QTest::newRow("9999") << 9999 << "9999" << "jpg";
}

void tst_StorageManager::fileNameGenerator()
{
    QFETCH(int, index);
    QFETCH(QString, idxString);
    QFETCH(QString, extension);

    StorageManager storage;
    storage.m_directory = "/tmp";
    const QLatin1String photoBase("image");

    QRegExp middlePattern("\\d\\d\\.\\d\\d\\.\\d\\d");
    QString date = QDate::currentDate().toString("yyyy.MM.dd");
    QString expectedPre = QString("/tmp/image%1").arg(date);
    QString expectedPost = QString("%1.%2").arg(idxString).arg(extension);

    QString generated = storage.fileNameGenerator(index, photoBase, extension);
    QStringList parts = generated.split('_');
    QCOMPARE(parts.count(), 3);

    QString pre = parts[0];
    QCOMPARE(pre, expectedPre);
    QString post = parts[2];
    QCOMPARE(post, expectedPost);
    QString middle = parts[1];
    QVERIFY(middlePattern.exactMatch(middle));
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
