/*
 * Copyright (C) 2012 Canonical, Ltd.
 *
 * Authors:
 *  Guenter Schwann <guenter.schwann@canonical.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QtTest/QtTest>

#define private public
#include "storagemanager.h"

class tst_StorageManager : public QObject
{
    Q_OBJECT
private slots:
    void fileNameGenerator_data();
    void fileNameGenerator();
};

void tst_StorageManager::fileNameGenerator_data()
{
    QTest::addColumn<int>("index");
    QTest::addColumn<QString>("idxString");
    QTest::addColumn<QString>("extension");

    QTest::newRow("1") << 1 << "0001" << "jpg";
    QTest::newRow("12") << 12 << "0012" << "mpg";
    QTest::newRow("9999") << 9999 << "9999" << "jpg";
}

void tst_StorageManager::fileNameGenerator()
{
    QFETCH(int, index);
    QFETCH(QString, idxString);
    QFETCH(QString, extension);

    StorageManager storage;

    QString date = QDate::currentDate().toString("yyyyMMdd");
    QString expected = QString("/tmp/image%1_%2.%3").arg(date).arg(idxString).arg(extension);
    QString generated = storage.fileNameGenerator(index, extension);
    QCOMPARE(generated, expected);
}

QTEST_MAIN(tst_StorageManager)

#include "tst_storagemanager.moc"
