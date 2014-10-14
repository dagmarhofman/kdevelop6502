/***************************************************************************
 *   This file was partly taken from KDevelop's cvs plugin                 *
 *   Copyright 2007 Robert Gruber <rgruber@users.sourceforge.net>          *
 *                                                                         *
 *   Adapted for Git                                                       *
 *   Copyright 2008 Evgeniy Ivanov <powerfox@kde.ru>                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU General Public License as        *
 *   published by the Free Software Foundation; either version 2 of        *
 *   the License or (at your option) version 3 or any later version        *
 *   accepted by the membership of KDE e.V. (or its successor approved     *
 *   by the membership of KDE e.V.), which shall act as a proxy            *
 *   defined in Section 14 of version 3 of the license.                    *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 ***************************************************************************/

#include "test_git.h"

#include <QtTest/QtTest>
#include <tests/testcore.h>
#include <tests/autotestshell.h>
#include <QUrl>
#include <QDebug>

#include <vcs/dvcs/dvcsjob.h>
#include <vcs/vcsannotation.h>
#include "../gitplugin.h"

#define VERIFYJOB(j) \
do { QVERIFY(j); QVERIFY(j->exec()); QVERIFY((j)->status() == KDevelop::VcsJob::JobSucceeded); } while(0)

const QString tempDir = QDir::tempPath();
const QString gitTest_BaseDir(tempDir + "/kdevGit_testdir/");
const QString gitTest_BaseDir2(tempDir + "/kdevGit_testdir2/");
const QString gitRepo(gitTest_BaseDir + ".git");
const QString gitSrcDir(gitTest_BaseDir + "src/");
const QString gitTest_FileName("testfile");
const QString gitTest_FileName2("foo");
const QString gitTest_FileName3("bar");

using namespace KDevelop;

void GitInitTest::initTestCase()
{
    AutoTestShell::init();
    TestCore::initialize(Core::NoUi);

    m_plugin = new GitPlugin(TestCore::self());
}

void GitInitTest::cleanupTestCase()
{
    delete m_plugin;

    TestCore::shutdown();
}

void GitInitTest::init()
{
    // Now create the basic directory structure
    QDir tmpdir(tempDir);
    tmpdir.mkdir(gitTest_BaseDir);
    tmpdir.mkdir(gitSrcDir);
    tmpdir.mkdir(gitTest_BaseDir2);
}

void GitInitTest::cleanup()
{
    removeTempDirs();
}


void GitInitTest::repoInit()
{
    qDebug() << "Trying to init repo";
    // make job that creates the local repository
    VcsJob* j = m_plugin->init(QUrl::fromLocalFile(gitTest_BaseDir));
    VERIFYJOB(j);

    //check if the CVSROOT directory in the new local repository exists now
    QVERIFY(QFileInfo(gitRepo).exists());

    //check if isValidDirectory works
    QVERIFY(m_plugin->isValidDirectory(QUrl::fromLocalFile(gitTest_BaseDir)));
    //and for non-git dir, I hope nobody has /tmp under git
    QVERIFY(!m_plugin->isValidDirectory(QUrl::fromLocalFile("/tmp")));

    //we have nothing, so output should be empty
    DVcsJob * j2 = m_plugin->gitRevParse(gitRepo, QStringList(QString("--branches")));
    QVERIFY(j2);
    QVERIFY(j2->exec());
    QString out = j2->output();
    QVERIFY(j2->output().isEmpty());
}

void GitInitTest::addFiles()
{
    qDebug() << "Adding files to the repo";

    //we start it after repoInit, so we still have empty git repo
    QFile f(gitTest_BaseDir + gitTest_FileName);

    if (f.open(QIODevice::WriteOnly)) {
        QTextStream input(&f);
        input << "HELLO WORLD";
    }

    f.close();
    f.setFileName(gitTest_BaseDir + gitTest_FileName2);

    if (f.open(QIODevice::WriteOnly)) {
        QTextStream input(&f);
        input << "No, bar()!";
    }

    f.close();

    //test git-status exitCode (see DVcsJob::setExitCode).
    VcsJob* j = m_plugin->status(QList<QUrl>() << QUrl::fromLocalFile(gitTest_BaseDir));
    VERIFYJOB(j);

    // /tmp/kdevGit_testdir/ and testfile
    j = m_plugin->add(QList<QUrl>() << QUrl::fromLocalFile(gitTest_BaseDir + gitTest_FileName));
    VERIFYJOB(j);

    f.setFileName(gitSrcDir + gitTest_FileName3);

    if (f.open(QIODevice::WriteOnly)) {
        QTextStream input(&f);
        input << "No, foo()! It's bar()!";
    }

    f.close();

    //test git-status exitCode again
    j = m_plugin->status(QList<QUrl>() << QUrl::fromLocalFile(gitTest_BaseDir));
    VERIFYJOB(j);

    //repository path without trailing slash and a file in a parent directory
    // /tmp/repo  and /tmp/repo/src/bar
    j = m_plugin->add(QList<QUrl>() << QUrl::fromLocalFile(gitSrcDir + gitTest_FileName3));
    VERIFYJOB(j);

    //let's use absolute path, because it's used in ContextMenus
    j = m_plugin->add(QList<QUrl>() << QUrl::fromLocalFile(gitTest_BaseDir + gitTest_FileName2));
    VERIFYJOB(j);

    //Now let's create several files and try "git add file1 file2 file3"
    QStringList files = QStringList() << "file1" << "file2" << "la la";
    QList<QUrl> multipleFiles;
    foreach(const QString& file, files) {
        QFile f(gitTest_BaseDir + file);
        QVERIFY(f.open(QIODevice::WriteOnly));
        QTextStream input(&f);
        input << file;
        f.close();
        multipleFiles << QUrl::fromLocalFile(gitTest_BaseDir + file);
    }
    j = m_plugin->add(multipleFiles);
    VERIFYJOB(j);
}

void GitInitTest::commitFiles()
{
    qDebug() << "Committing...";
    //we start it after addFiles, so we just have to commit
    VcsJob* j = m_plugin->commit(QString("Test commit"), QList<QUrl>() << QUrl::fromLocalFile(gitTest_BaseDir));
    VERIFYJOB(j);

    //test git-status exitCode one more time.
    j = m_plugin->status(QList<QUrl>() << QUrl::fromLocalFile(gitTest_BaseDir));
    VERIFYJOB(j);

    //since we committed the file to the "pure" repository, .git/refs/heads/master should exist
    //TODO: maybe other method should be used
    QString headRefName(gitRepo + "/refs/heads/master");
    QVERIFY(QFileInfo(headRefName).exists());

    //Test the results of the "git add"
    DVcsJob* jobLs = new DVcsJob(gitTest_BaseDir, m_plugin);
    *jobLs << "git" << "ls-tree" << "--name-only" << "-r" << "HEAD";

    if (jobLs->exec() && jobLs->status() == KDevelop::VcsJob::JobSucceeded) {
        QStringList files = jobLs->output().split('\n');
        QVERIFY(files.contains(gitTest_FileName));
        QVERIFY(files.contains(gitTest_FileName2));
        QVERIFY(files.contains("src/" + gitTest_FileName3));
    }

    QString firstCommit;

    QFile headRef(headRefName);

    if (headRef.open(QIODevice::ReadOnly)) {
        QTextStream output(&headRef);
        output >> firstCommit;
    }
    headRef.close();

    QVERIFY(!firstCommit.isEmpty());

    qDebug() << "Committing one more time";
    //let's try to change the file and test "git commit -a"
    QFile f(gitTest_BaseDir + gitTest_FileName);

    if (f.open(QIODevice::WriteOnly)) {
        QTextStream input(&f);
        input << "Just another HELLO WORLD\n";
    }

    f.close();

    //add changes
    j = m_plugin->add(QList<QUrl>() << QUrl::fromLocalFile(gitTest_BaseDir + gitTest_FileName));
    VERIFYJOB(j);

    j = m_plugin->commit(QString("KDevelop's Test commit2"), QList<QUrl>() << QUrl::fromLocalFile(gitTest_BaseDir));
    VERIFYJOB(j);

    QString secondCommit;

    if (headRef.open(QIODevice::ReadOnly)) {
        QTextStream output(&headRef);
        output >> secondCommit;
    }
    headRef.close();

    QVERIFY(!secondCommit.isEmpty());
    QVERIFY(firstCommit != secondCommit);

}

void GitInitTest::testInit()
{
    repoInit();
}

void GitInitTest::testAdd()
{
    repoInit();
    addFiles();
}

void GitInitTest::testCommit()
{
    repoInit();
    addFiles();
    commitFiles();
}

void GitInitTest::testBranch(const QString& newBranch)
{
    //Already tested, so I assume that it works
    const QUrl baseUrl = QUrl::fromLocalFile(gitTest_BaseDir);
    QString oldBranch = runSynchronously(m_plugin->currentBranch(baseUrl)).toString();

    VcsRevision rev;
    rev.setRevisionValue(oldBranch, KDevelop::VcsRevision::GlobalNumber);
    VcsJob* j = m_plugin->branch(baseUrl, rev, newBranch);
    VERIFYJOB(j);
    QVERIFY(runSynchronously(m_plugin->branches(baseUrl)).toStringList().contains(newBranch));

    // switch branch
    j = m_plugin->switchBranch(baseUrl, newBranch);
    VERIFYJOB(j);
    QCOMPARE(runSynchronously(m_plugin->currentBranch(baseUrl)).toString(), newBranch);

    // get into detached head state
    j = m_plugin->switchBranch(baseUrl, "HEAD~1");
    VERIFYJOB(j);
    QCOMPARE(runSynchronously(m_plugin->currentBranch(baseUrl)).toString(), QString(""));

    // switch back
    j = m_plugin->switchBranch(baseUrl, newBranch);
    VERIFYJOB(j);
    QCOMPARE(runSynchronously(m_plugin->currentBranch(baseUrl)).toString(), newBranch);

    j = m_plugin->deleteBranch(baseUrl, oldBranch);
    VERIFYJOB(j);
    QVERIFY(!runSynchronously(m_plugin->branches(baseUrl)).toStringList().contains(oldBranch));
}

void GitInitTest::testBranching()
{
    repoInit();
    addFiles();
    commitFiles();

    const QUrl baseUrl = QUrl::fromLocalFile(gitTest_BaseDir);
    VcsJob* j = m_plugin->branches(baseUrl);
    VERIFYJOB(j);

    QString curBranch = runSynchronously(m_plugin->currentBranch(baseUrl)).toString();
    QCOMPARE(curBranch, QString("master"));

    testBranch("new");
    testBranch("averylongbranchnamejusttotestlongnames");
    testBranch("KDE/4.10");
}

void GitInitTest::revHistory()
{
    repoInit();
    addFiles();
    commitFiles();

    QList<DVcsEvent> commits = m_plugin->getAllCommits(gitTest_BaseDir);
    QVERIFY(!commits.isEmpty());
    QStringList logMessages;

    for (int i = 0; i < commits.count(); ++i)
        logMessages << commits[i].getLog();

    QCOMPARE(commits.count(), 2);

    QCOMPARE(logMessages[0], QString("KDevelop's Test commit2"));  //0 is later than 1!

    QCOMPARE(logMessages[1], QString("Test commit"));

    QVERIFY(commits[1].getParents().isEmpty());  //0 is later than 1!

    QVERIFY(!commits[0].getParents().isEmpty()); //initial commit is on the top

    QVERIFY(commits[1].getCommit().contains(QRegExp("^\\w{,40}$")));

    QVERIFY(commits[0].getCommit().contains(QRegExp("^\\w{,40}$")));

    QVERIFY(commits[0].getParents()[0].contains(QRegExp("^\\w{,40}$")));
}

void GitInitTest::testAnnotation()
{
    repoInit();
    addFiles();
    commitFiles();

    // called after commitFiles
    QFile f(gitTest_BaseDir + gitTest_FileName);
    QVERIFY(f.open(QIODevice::Append));
    QTextStream input(&f);
    input << "An appended line";
    f.close();

    VcsJob* j = m_plugin->commit(QString("KDevelop's Test commit3"), QList<QUrl>() << QUrl::fromLocalFile(gitTest_BaseDir));
    VERIFYJOB(j);

    j = m_plugin->annotate(QUrl::fromLocalFile(gitTest_BaseDir + gitTest_FileName), VcsRevision::createSpecialRevision(VcsRevision::Head));
    VERIFYJOB(j);

    QList<QVariant> results = j->fetchResults().toList();
    QCOMPARE(results.size(), 2);
    QVERIFY(results.at(0).canConvert<VcsAnnotationLine>());
    VcsAnnotationLine annotation = results.at(0).value<VcsAnnotationLine>();
    QCOMPARE(annotation.lineNumber(), 0);
    QCOMPARE(annotation.commitMessage(), QString("KDevelop's Test commit2"));

    QVERIFY(results.at(1).canConvert<VcsAnnotationLine>());
    annotation = results.at(1).value<VcsAnnotationLine>();
    QCOMPARE(annotation.lineNumber(), 1);
    QCOMPARE(annotation.commitMessage(), QString("KDevelop's Test commit3"));
}

void GitInitTest::testRemoveEmptyFolder()
{
    repoInit();

    QDir d(gitTest_BaseDir);
    d.mkdir("emptydir");

    VcsJob* j = m_plugin->remove(QList<QUrl>() << QUrl::fromLocalFile(gitTest_BaseDir+"emptydir/"));
    if (j) VERIFYJOB(j);

    QVERIFY(!d.exists("emptydir"));
}

void GitInitTest::testRemoveEmptyFolderInFolder()
{
    repoInit();

    QDir d(gitTest_BaseDir);
    d.mkdir("dir");

    QDir d2(gitTest_BaseDir+"dir");
    d2.mkdir("emptydir");

    VcsJob* j = m_plugin->remove(QList<QUrl>() << QUrl::fromLocalFile(gitTest_BaseDir+"dir/"));
    if (j) VERIFYJOB(j);

    QVERIFY(!d.exists("dir"));
}

void GitInitTest::testRemoveUnindexedFile()
{
    repoInit();

    QFile f(gitTest_BaseDir + gitTest_FileName);
    QVERIFY(f.open(QIODevice::Append));
    QTextStream input(&f);
    input << "An appended line";
    f.close();

    VcsJob* j = m_plugin->remove(QList<QUrl>() << QUrl::fromLocalFile(gitTest_BaseDir + gitTest_FileName));
    if (j) VERIFYJOB(j);

    QVERIFY(!QFile::exists(gitTest_BaseDir + gitTest_FileName));
}

void GitInitTest::testRemoveFolderContainingUnversionedFiles()
{
    repoInit();

    QDir d(gitTest_BaseDir);
    d.mkdir("dir");

    {
        QFile f(gitTest_BaseDir + "dir/foo");
        QVERIFY(f.open(QIODevice::Append));
        QTextStream input(&f);
        input << "An appended line";
        f.close();
    }
    VcsJob* j = m_plugin->add(QList<QUrl>() << QUrl::fromLocalFile(gitTest_BaseDir+"dir"));
    VERIFYJOB(j);
    j = m_plugin->commit("initial commit", QList<QUrl>() << QUrl::fromLocalFile(gitTest_BaseDir));
    VERIFYJOB(j);

    {
        QFile f(gitTest_BaseDir + "dir/bar");
        QVERIFY(f.open(QIODevice::Append));
        QTextStream input(&f);
        input << "An appended line";
        f.close();
    }

    j = m_plugin->remove(QList<QUrl>() << QUrl::fromLocalFile(gitTest_BaseDir + "dir"));
    if (j) VERIFYJOB(j);

    QVERIFY(!QFile::exists(gitTest_BaseDir + "dir"));

}


void GitInitTest::removeTempDirs()
{
    for (const auto& dirPath : {gitTest_BaseDir, gitTest_BaseDir2}) {
        QDir dir(dirPath);
        if (dir.exists() && !dir.removeRecursively()) {
            qDebug() << "QDir::removeRecursively(" << dirPath << ") returned false";
        }
    }
}

QTEST_MAIN(GitInitTest)

// #include "gittest.moc"
