/***************************************************************************
 *   Copyright (C) 2008 by Andreas Pakulat <apaku@gmx.de                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "openprojectpage.h"

#include <QtCore/QDir>
#include <QtCore/QModelIndex>
#include <QVBoxLayout>
#include <QHeaderView>

#include <kconfiggroup.h>
#include <kurlcompletion.h>
#include <kurlcombobox.h>
#include <kurlpixmapprovider.h>
#include <kfileitem.h>
#include <ksharedconfig.h>
#include <kplugininfo.h>
#include <kfilewidget.h>
#include <KLocalizedString>

#include "shellextension.h"
#include "core.h"
#include "plugincontroller.h"
#include <kdiroperator.h>
#include <kactioncollection.h>
#include <KIO/NetAccess>

namespace KDevelop
{

OpenProjectPage::OpenProjectPage( const QUrl& startUrl, QWidget* parent )
        : QWidget( parent )
{
    QHBoxLayout* layout = new QHBoxLayout( this );

    fileWidget = new KFileWidget( startUrl, this);

    QStringList filters;
    QStringList allEntry;
    allEntry << "*."+ShellExtension::getInstance()->projectFileExtension();
    filters << QString( "%1|%2 (%1)").arg("*."+ShellExtension::getInstance()->projectFileExtension()).arg(ShellExtension::getInstance()->projectFileDescription());
    foreach(const KPluginInfo& info, ICore::self()->pluginController()->queryExtensionPlugins( "org.kdevelop.IProjectFileManager" ) )
    {
        QVariant filter = info.property("X-KDevelop-ProjectFilesFilter");
	    QVariant desc = info.property("X-KDevelop-ProjectFilesFilterDescription");
        QString filterline;
        if( filter.isValid() && desc.isValid() )
        {
            m_projectFilters.insert( info.name(), filter.toStringList() );
            allEntry += filter.toStringList();
            filters << QString("%1|%2 (%1)").arg(filter.toStringList().join(" ")).arg(desc.toString());
        }
    }

    filters.prepend( i18n( "%1|All Project Files (%1)", allEntry.join(" ") ) );

    fileWidget->setFilter( filters.join("\n") );

    fileWidget->setMode( KFile::Modes( KFile::File | KFile::Directory | KFile::ExistingOnly ) );

    layout->addWidget( fileWidget );

    QWidget* ops= fileWidget->findChild<QWidget*>( "KFileWidget::ops" );
    // Emitted for changes in the places view, the url navigator and when using the back/forward/up buttons
    connect( ops, SIGNAL(urlEntered(QUrl)), SLOT(opsEntered(QUrl)));

    // Emitted when selecting an entry from the "Name" box or editing in there
    connect( fileWidget->locationEdit(), SIGNAL(editTextChanged(QString)),
             SLOT(comboTextChanged(QString)));

    // Emitted when clicking on a file in the fileview area
    connect( fileWidget, SIGNAL(fileHighlighted(QString)), SLOT(highlightFile(QString)) );

    connect( fileWidget->dirOperator()->dirLister(), SIGNAL(completed(QUrl)), SLOT(dirChanged(QUrl)));

    connect( fileWidget, SIGNAL(accepted()), SIGNAL(accepted()));
}

QUrl OpenProjectPage::getAbsoluteUrl( const QString& file ) const
{
    QUrl u(file);
    if( u.isRelative() )
    {
        u = fileWidget->baseUrl().resolved( u );
    }
    return u;
}

void OpenProjectPage::setUrl(const QUrl& url)
{
    fileWidget->setUrl(url, false);
}

void OpenProjectPage::dirChanged(const QUrl& /*url*/)
{
    if(fileWidget->selectedFiles().isEmpty()) {
        KFileItemList items=fileWidget->dirOperator()->dirLister()->items();
        foreach(const KFileItem& item, items) {
            if(item.url().path().endsWith(ShellExtension::getInstance()->projectFileExtension()) && item.isFile())
                fileWidget->setSelection(item.url().url());
        }
    }
}

void OpenProjectPage::showEvent(QShowEvent* ev)
{
    fileWidget->locationEdit()->setFocus();
    QWidget::showEvent(ev);
}

void OpenProjectPage::highlightFile( const QString& file )
{
    emit urlSelected( getAbsoluteUrl( file ) );
}

void OpenProjectPage::opsEntered( const QUrl& url )
{
    emit urlSelected( getAbsoluteUrl( url.url() ) );
}

void OpenProjectPage::comboTextChanged( const QString& file )
{
    emit urlSelected( getAbsoluteUrl( file ) );
}

QMap<QString,QStringList> OpenProjectPage::projectFilters() const
{
    return m_projectFilters;
}

}

