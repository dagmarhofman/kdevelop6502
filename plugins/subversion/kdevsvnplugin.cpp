/***************************************************************************
 *   Copyright 2007 Dukju Ahn <dukjuahn@gmail.com>                         *
 *   Copyright 2008 Andreas Pakulat <apaku@gmx.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kdevsvnplugin.h"

#include <QAction>
#include <QVariant>
#include <QTextStream>
#include <QMenu>

#include <kparts/part.h>
#include <kparts/partmanager.h>
#include <kparts/mainwindow.h>
#include <kaboutdata.h>
#include <ktexteditor/document.h>
#include <ktexteditor/markinterface.h>
#include <kpluginfactory.h>
#include <kpluginloader.h>
#include <KLocalizedString>
#include <kurlrequester.h>
#include <kaction.h>
#include <kurlrequesterdialog.h>
#include <kfile.h>
#include <ktemporaryfile.h>
#include <kmessagebox.h>
#include <kfiledialog.h>

#include <interfaces/iuicontroller.h>
#include <interfaces/idocumentcontroller.h>
#include <interfaces/idocument.h>
#include <interfaces/iplugincontroller.h>
#include <interfaces/icore.h>
#include <interfaces/iruncontroller.h>
#include <project/projectmodel.h>
#include <interfaces/context.h>
#include <interfaces/contextmenuextension.h>
#include <vcs/vcsrevision.h>
#include <vcs/vcsevent.h>
#include <vcs/vcsstatusinfo.h>
#include <vcs/vcsannotation.h>
#include <vcs/widgets/vcseventwidget.h>
#include <vcs/widgets/vcsdiffwidget.h>
#include <vcs/widgets/vcscommitdialog.h>
#include <language/interfaces/editorcontext.h>

#include "kdevsvncpp/apr.hpp"

#include "svncommitjob.h"
#include "svnstatusjob.h"
#include "svnaddjob.h"
#include "svnrevertjob.h"
#include "svnremovejob.h"
#include "svnupdatejob.h"
#include "svninfojob.h"
#include "svndiffjob.h"
#include "svncopyjob.h"
#include "svnmovejob.h"
#include "svnlogjob.h"
#include "svnblamejob.h"
#include "svnimportjob.h"
#include "svncheckoutjob.h"

#include "svnimportmetadatawidget.h"
#include "svncheckoutmetadatawidget.h"
#include <vcs/vcspluginhelper.h>
#include <vcs/widgets/standardvcslocationwidget.h>
#include "svnlocationwidget.h"
#include "debug.h"

Q_LOGGING_CATEGORY(PLUGIN_SVN, "kdevplatform.plugins.svn")
K_PLUGIN_FACTORY_WITH_JSON(KDevSvnFactory, "kdevsubversion.json", registerPlugin<KDevSvnPlugin>();)

KDevSvnPlugin::KDevSvnPlugin(QObject *parent, const QVariantList &)
        : KDevelop::IPlugin("kdevsubversion", parent)
        , m_common(new KDevelop::VcsPluginHelper(this, this)),
        copy_action( 0 ), move_action( 0 )
{
    KDEV_USE_EXTENSION_INTERFACE(KDevelop::IBasicVersionControl)
    KDEV_USE_EXTENSION_INTERFACE(KDevelop::ICentralizedVersionControl)

    qRegisterMetaType<KDevelop::VcsStatusInfo>();
    qRegisterMetaType<SvnInfoHolder>();
    qRegisterMetaType<KDevelop::VcsEvent>();
    qRegisterMetaType<KDevelop::VcsRevision>();
    qRegisterMetaType<KDevelop::VcsRevision::RevisionSpecialType>();
    qRegisterMetaType<KDevelop::VcsAnnotation>();
    qRegisterMetaType<KDevelop::VcsAnnotationLine>();
}

KDevSvnPlugin::~KDevSvnPlugin()
{
}

bool KDevSvnPlugin::isVersionControlled(const QUrl &localLocation)
{
    ///TODO: also check this in the other functions?
    if (!localLocation.isValid()) {
        return false;
    }

    SvnInfoJob* job = new SvnInfoJob(this);

    job->setLocation(localLocation);

    if (job->exec()) {
        QVariant result = job->fetchResults();

        if (result.isValid()) {
            SvnInfoHolder h = result.value<SvnInfoHolder>();
            return !h.name.isEmpty();
        }
    } else {
        qCDebug(PLUGIN_SVN) << "Couldn't execute job";
    }

    return false;
}

KDevelop::VcsJob* KDevSvnPlugin::repositoryLocation(const QUrl &localLocation)
{
    SvnInfoJob* job = new SvnInfoJob(this);

    job->setLocation(localLocation);
    job->setProvideInformation(SvnInfoJob::RepoUrlOnly);
    return job;
}

KDevelop::VcsJob* KDevSvnPlugin::status(const QList<QUrl>& localLocations,
                                        KDevelop::IBasicVersionControl::RecursionMode mode)
{
    SvnStatusJob* job = new SvnStatusJob(this);
    job->setLocations(localLocations);
    job->setRecursive((mode == KDevelop::IBasicVersionControl::Recursive));
    return job;
}

KDevelop::VcsJob* KDevSvnPlugin::add(const QList<QUrl>& localLocations,
                                     KDevelop::IBasicVersionControl::RecursionMode recursion)
{
    SvnAddJob* job = new SvnAddJob(this);
    job->setLocations(localLocations);
    job->setRecursive((recursion == KDevelop::IBasicVersionControl::Recursive));
    return job;
}

KDevelop::VcsJob* KDevSvnPlugin::remove(const QList<QUrl>& localLocations)
{
    SvnRemoveJob* job = new SvnRemoveJob(this);
    job->setLocations(localLocations);
    return job;
}

KDevelop::VcsJob* KDevSvnPlugin::edit(const QUrl& /*localLocation*/)
{
    return 0;
}

KDevelop::VcsJob* KDevSvnPlugin::unedit(const QUrl& /*localLocation*/)
{
    return 0;
}

KDevelop::VcsJob* KDevSvnPlugin::localRevision(const QUrl &localLocation, KDevelop::VcsRevision::RevisionType type)
{
    SvnInfoJob* job = new SvnInfoJob(this);

    job->setLocation(localLocation);
    job->setProvideInformation(SvnInfoJob::RevisionOnly);
    job->setProvideRevisionType(type);
    return job;
}

KDevelop::VcsJob* KDevSvnPlugin::copy(const QUrl &localLocationSrc, const QUrl& localLocationDstn)
{
    SvnCopyJob* job = new SvnCopyJob(this);
    job->setSourceLocation(localLocationSrc);
    job->setDestinationLocation(localLocationDstn);
    return job;
}

KDevelop::VcsJob* KDevSvnPlugin::move(const QUrl &localLocationSrc, const QUrl& localLocationDst)
{
    SvnMoveJob* job = new SvnMoveJob(this);
    job->setSourceLocation(localLocationSrc);
    job->setDestinationLocation(localLocationDst);
    return job;
}

KDevelop::VcsJob* KDevSvnPlugin::revert(const QList<QUrl>& localLocations,
                                        KDevelop::IBasicVersionControl::RecursionMode recursion)
{
    SvnRevertJob* job = new SvnRevertJob(this);
    job->setLocations(localLocations);
    job->setRecursive((recursion == KDevelop::IBasicVersionControl::Recursive));
    return job;
}

KDevelop::VcsJob* KDevSvnPlugin::update(const QList<QUrl>& localLocations,
                                        const KDevelop::VcsRevision& rev,
                                        KDevelop::IBasicVersionControl::RecursionMode recursion)
{
    SvnUpdateJob* job = new SvnUpdateJob(this);
    job->setLocations(localLocations);
    job->setRevision(rev);
    job->setRecursive((recursion == KDevelop::IBasicVersionControl::Recursive));
    return job;
}

KDevelop::VcsJob* KDevSvnPlugin::commit(const QString& message, const QList<QUrl>& localLocations,
                                        KDevelop::IBasicVersionControl::RecursionMode recursion)
{
    SvnCommitJob* job = new SvnCommitJob(this);
    qCDebug(PLUGIN_SVN) << "Committing locations:" << localLocations << endl;
    job->setUrls(localLocations);
    job->setCommitMessage(message) ;
    job->setRecursive((recursion == KDevelop::IBasicVersionControl::Recursive));
    return job;
}

KDevelop::VcsJob* KDevSvnPlugin::diff(const QUrl &fileOrDirectory,
                                      const KDevelop::VcsRevision& srcRevision,
                                      const KDevelop::VcsRevision& dstRevision,
                                      KDevelop::VcsDiff::Type diffType,
                                      KDevelop::IBasicVersionControl::RecursionMode recurse)
{
    KDevelop::VcsLocation loc(fileOrDirectory);
    return diff2(loc, loc, srcRevision, dstRevision, diffType, recurse);
}

KDevelop::VcsJob* KDevSvnPlugin::diff2(const KDevelop::VcsLocation& src,
                                       const KDevelop::VcsLocation& dst,
                                       const KDevelop::VcsRevision& srcRevision,
                                       const KDevelop::VcsRevision& dstRevision,
                                       KDevelop::VcsDiff::Type diffType,
                                       KDevelop::IBasicVersionControl::RecursionMode recurse)
{
    SvnDiffJob* job = new SvnDiffJob(this);
    job->setSource(src);
    job->setDestination(dst);
    job->setSrcRevision(srcRevision);
    job->setDstRevision(dstRevision);
    job->setDiffType(diffType);
    job->setRecursive((recurse == KDevelop::IBasicVersionControl::Recursive));
    return job;
}

KDevelop::VcsJob* KDevSvnPlugin::log(const QUrl &localLocation, const KDevelop::VcsRevision& rev, unsigned long limit)
{
    SvnLogJob* job = new SvnLogJob(this);
    job->setLocation(localLocation);
    job->setStartRevision(rev);
    job->setLimit(limit);
    return job;
}

KDevelop::VcsJob* KDevSvnPlugin::log(const QUrl &localLocation,
                                     const KDevelop::VcsRevision& startRev,
                                     const KDevelop::VcsRevision& endRev)
{
    SvnLogJob* job = new SvnLogJob(this);
    job->setLocation(localLocation);
    job->setStartRevision(startRev);
    job->setEndRevision(endRev);
    return job;
}

KDevelop::VcsJob* KDevSvnPlugin::annotate(const QUrl &localLocation,
        const KDevelop::VcsRevision& rev)
{
    SvnBlameJob* job = new SvnBlameJob(this);
    job->setLocation(localLocation);
    job->setEndRevision(rev);
    return job;
}

KDevelop::VcsJob* KDevSvnPlugin::merge(const KDevelop::VcsLocation& localOrRepoLocationSrc,
                                       const KDevelop::VcsLocation& localOrRepoLocationDst,
                                       const KDevelop::VcsRevision& srcRevision,
                                       const KDevelop::VcsRevision& dstRevision,
                                       const QUrl &localLocation)
{
    // TODO implement merge
    Q_UNUSED(localOrRepoLocationSrc)
    Q_UNUSED(localOrRepoLocationDst)
    Q_UNUSED(srcRevision)
    Q_UNUSED(dstRevision)
    Q_UNUSED(localLocation)
    return 0;
}

KDevelop::VcsJob* KDevSvnPlugin::resolve(const QList<QUrl>& /*localLocations*/,
        KDevelop::IBasicVersionControl::RecursionMode /*recursion*/)
{
    return 0;
}

KDevelop::VcsJob* KDevSvnPlugin::import(const QString & commitMessage, const QUrl &sourceDirectory, const KDevelop::VcsLocation & destinationRepository)
{
    SvnImportJob* job = new SvnImportJob(this);
    job->setMapping(sourceDirectory, destinationRepository);
    job->setMessage(commitMessage);
    return job;
}

KDevelop::VcsJob* KDevSvnPlugin::createWorkingCopy(const KDevelop::VcsLocation & sourceRepository, const QUrl &destinationDirectory, KDevelop::IBasicVersionControl::RecursionMode recursion)
{
    SvnCheckoutJob* job = new SvnCheckoutJob(this);
    job->setMapping(sourceRepository, destinationDirectory, recursion);
    return job;
}


KDevelop::ContextMenuExtension KDevSvnPlugin::contextMenuExtension(KDevelop::Context* context)
{
    m_common->setupFromContext(context);

    const QList<QUrl> & ctxUrlList  = m_common->contextUrlList();

    bool hasVersionControlledEntries = false;
    foreach(const QUrl &url, ctxUrlList) {
        if (isVersionControlled(url) || isVersionControlled(url.upUrl())) {
            hasVersionControlledEntries = true;
            break;
        }
    }

    qCDebug(PLUGIN_SVN) << "version controlled?" << hasVersionControlledEntries;

    if (!hasVersionControlledEntries)
        return IPlugin::contextMenuExtension(context);


    QMenu* svnmenu= m_common->commonActions();
    svnmenu->addSeparator();

    if( !copy_action )
    {
        copy_action = new QAction(i18n("Copy..."), this);
        connect(copy_action, SIGNAL(triggered()), this, SLOT(ctxCopy()));
    }
    svnmenu->addAction(copy_action);

    if( !move_action )
    {
        move_action = new QAction(i18n("Move..."), this);
        connect(move_action, SIGNAL(triggered()), this, SLOT(ctxMove()));
    }
    svnmenu->addAction(move_action);

    KDevelop::ContextMenuExtension menuExt;
    menuExt.addAction(KDevelop::ContextMenuExtension::VcsGroup, svnmenu->menuAction());

    return menuExt;
}

void KDevSvnPlugin::ctxInfo()
{
    QList<QUrl> const & ctxUrlList = m_common->contextUrlList();
    if (ctxUrlList.count() != 1) {
        KMessageBox::error(0, i18n("Please select only one item for this operation"));
        return;
    }
}

void KDevSvnPlugin::ctxStatus()
{
    QList<QUrl> const & ctxUrlList = m_common->contextUrlList();
    if (ctxUrlList.count() > 1) {
        KMessageBox::error(0, i18n("Please select only one item for this operation"));
        return;
    }
}

void KDevSvnPlugin::ctxCopy()
{
    QList<QUrl> const & ctxUrlList = m_common->contextUrlList();
    if (ctxUrlList.count() > 1) {
        KMessageBox::error(0, i18n("Please select only one item for this operation"));
        return;
    }

    QUrl source = ctxUrlList.first();

    if (source.isLocalFile()) {
        QString dir = source.toLocalFile();
        bool isFile = QFileInfo(source.toLocalFile()).isFile();

        if (isFile) {
            dir = source.adjusted(QUrl::RemoveFilename|QUrl::StripTrailingSlash).path();
        }

        KUrlRequesterDialog dlg(dir, i18n("Destination file/directory"), 0);

        if (isFile) {
            dlg.urlRequester()->setMode(KFile::File | KFile::Directory | KFile::LocalOnly);
        } else {
            dlg.urlRequester()->setMode(KFile::Directory | KFile::LocalOnly);
        }

        if (dlg.exec() == QDialog::Accepted) {
            KDevelop::ICore::self()->runController()->registerJob(copy(source, dlg.selectedUrl()));
        }
    } else {
        KMessageBox::error(0, i18n("Copying only works on local files"));
        return;
    }

}

void KDevSvnPlugin::ctxMove()
{
    QList<QUrl> const & ctxUrlList = m_common->contextUrlList();
    if (ctxUrlList.count() != 1) {
        KMessageBox::error(0, i18n("Please select only one item for this operation"));
        return;
    }

    QUrl source = ctxUrlList.first();

    if (source.isLocalFile()) {
        QString dir = source.toLocalFile();
        bool isFile = QFileInfo(source.toLocalFile()).isFile();

        if (isFile) {
            dir = source.adjusted(QUrl::RemoveFilename|QUrl::StripTrailingSlash).path();
        }

        KUrlRequesterDialog dlg(dir, i18n("Destination file/directory"), 0);

        dlg.fileDialog()->setAcceptMode(QFileDialog::AcceptSave);
        if (isFile) {
            dlg.urlRequester()->setMode(KFile::File | KFile::Directory | KFile::LocalOnly);
        } else {
            dlg.urlRequester()->setMode(KFile::Directory | KFile::LocalOnly);
        }

        if (dlg.exec() == QDialog::Accepted) {
            KDevelop::ICore::self()->runController()->registerJob(move(source, dlg.selectedUrl()));
        }
    } else {
        KMessageBox::error(0, i18n("Moving only works on local files/dirs"));
        return;
    }
}

void KDevSvnPlugin::ctxCat()
{
    QList<QUrl> const & ctxUrlList = m_common->contextUrlList();
    if (ctxUrlList.count() != 1) {
        KMessageBox::error(0, i18n("Please select only one item for this operation"));
        return;
    }
}

QString KDevSvnPlugin::name() const
{
    return i18n("Subversion");
}

KDevelop::VcsImportMetadataWidget* KDevSvnPlugin::createImportMetadataWidget(QWidget* parent)
{
    return new SvnImportMetadataWidget(parent);
}

void KDevSvnPlugin::ctxImport()
{
    QList<QUrl> const & ctxUrlList = m_common->contextUrlList();
    if (ctxUrlList.count() != 1) {
        KMessageBox::error(0, i18n("Please select only one item for this operation"));
        return;
    }

    KDialog dlg;

    dlg.setCaption(i18n("Import into Subversion repository"));
    SvnImportMetadataWidget* widget = new SvnImportMetadataWidget(&dlg);
    widget->setSourceLocation(KDevelop::VcsLocation(ctxUrlList.first()));
    widget->setSourceLocationEditable(false);
    dlg.setMainWidget(widget);

    if (dlg.exec() == QDialog::Accepted) {
        KDevelop::ICore::self()->runController()->registerJob(import(widget->message(), widget->source(), widget->destination()));
    }
}

void KDevSvnPlugin::ctxCheckout()
{
    QList<QUrl> const & ctxUrlList = m_common->contextUrlList();
    if (ctxUrlList.count() != 1) {
        KMessageBox::error(0, i18n("Please select only one item for this operation"));
        return;
    }

    KDialog dlg;

    dlg.setCaption(i18n("Checkout from Subversion repository"));
    SvnCheckoutMetadataWidget* widget = new SvnCheckoutMetadataWidget(&dlg);
    QUrl tmp = ctxUrlList.first();
    tmp.cd("..");
    widget->setDestinationLocation(tmp);
    dlg.setMainWidget(widget);

    if (dlg.exec() == QDialog::Accepted) {
        KDevelop::ICore::self()->runController()->registerJob(createWorkingCopy(widget->source(), widget->destination(), widget->recursionMode()));
    }
}

KDevelop::VcsLocationWidget* KDevSvnPlugin::vcsLocation(QWidget* parent) const
{
    return new SvnLocationWidget(parent);
}

#include "kdevsvnplugin.moc"
