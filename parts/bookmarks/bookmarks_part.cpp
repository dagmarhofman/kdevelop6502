
#include <qwhatsthis.h>

#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <ktexteditor/markinterface.h>
#include <ktexteditor/document.h>
#include <kaction.h>

#include <kdevpartcontroller.h>
#include "kdevcore.h"
#include "kdevmainwindow.h"


#include "bookmarks_widget.h"
#include "bookmarks_part.h"


typedef KGenericFactory<BookmarksPart> BookmarksFactory;
K_EXPORT_COMPONENT_FACTORY( libkdevbookmarks, BookmarksFactory( "kdevbookmarks" ) );

BookmarksPart::BookmarksPart(QObject *parent, const char *name, const QStringList& )
	: KDevPlugin("bookmarks", "bookmarks", parent, name ? name : "BookmarksPart" )
{
	setInstance(BookmarksFactory::instance());
//	setXMLFile("kdevpart_bookmarks.rc");

	_widget = new BookmarksWidget(this);

	_widget->setCaption(i18n("Bookmarks"));
	_widget->setIcon(SmallIcon("bookmark"));

	QWhatsThis::add(_widget, i18n("Bookmarks\n\n"
			"The bookmark viewer shows all the source bookmarks in the project."));

	mainWindow()->embedSelectView(_widget, i18n("Bookmarks"), i18n("source bookmarks in the project"));

//	KAction * action = new KAction( i18n("Testing bookmarks..."), CTRL+ALT+Key_L, this,
//		SLOT( marksChanged() ), actionCollection(), "bookmarks" );


	// ===================

	_editorMap.setAutoDelete( true );

	_settingMarks = false;

	connect( core(), SIGNAL( projectOpened() ), this, SLOT( projectOpened() ) );
	connect( core(), SIGNAL( projectClosed() ), this, SLOT( projectClosed() ) );

	connect( partController(), SIGNAL( partAdded( KParts::Part * ) ), this, SLOT( partAdded( KParts::Part * ) ) );

	// load bookmarks from file

}


BookmarksPart::~BookmarksPart()
{
	delete _widget;
}

void BookmarksPart::partAdded( KParts::Part * part )
{
	kdDebug(0) << "BookmarksPart::partAdded()" << endl;

	if ( KParts::ReadOnlyPart * ro_part = dynamic_cast<KParts::ReadOnlyPart *>(part) )
	{
		if ( KTextEditor::MarkInterface * mi = dynamic_cast<KTextEditor::MarkInterface *>(ro_part) )
		{
			if ( EditorData * data = _editorMap.find( ro_part->url().path() ) )
			{
				// we've seen this one before, apply stored bookmarks
				_settingMarks = true;

				QValueListIterator<int> it = data->marks.begin();
				while ( it != data->marks.end() )
				{
					kdDebug(0) << "Setting bookmark. Line: " << *it << endl;
					mi->addMark( *it, KTextEditor::MarkInterface::markType01 );
					++it;
				}

				_settingMarks = false;
			}

			// connect to this editor
			KTextEditor::Document * doc = static_cast<KTextEditor::Document*>( ro_part );
			connect( doc, SIGNAL( marksChanged() ), this, SLOT( marksChanged() ) );
		}
	}
}

void BookmarksPart::marksChanged()
{
	kdDebug(0) << "BookmarksPart::marksChanged()" << endl;

    QObject * senderobj = const_cast<QObject*>( sender() );
	KParts::ReadOnlyPart * ro_part = dynamic_cast<KParts::ReadOnlyPart *>( senderobj );
	KTextEditor::MarkInterface * mi = dynamic_cast<KTextEditor::MarkInterface*>( senderobj );

	// don't react if we're in the middle of setting marks
	if ( ! _settingMarks )
	{
		if ( ro_part && mi )
		{
			kdDebug(0) << "found a MarkInterface" << endl;

			if ( EditorData * data = storeBookmarksForURL( ro_part ) )
			{
				_widget->updateURL( data );
			}
            else
            {
                _widget->removeURL( ro_part->url() );
            }
		}
		else
		{
			kdDebug(0) << "ReadOnlyPart == " << ro_part << endl
				<< "MarkInterface == " << mi << endl
				<< "examining all loaded parts instead" << endl;

			examineLoadedParts();
			_widget->update( _editorMap );

			return;
		}
	}
	else
	{
		kdDebug(0) << "currently setting marks, bailing out" << endl;
	}
}

void BookmarksPart::projectOpened()
{
	kdDebug(0) << "BookmarksPart::projectOpened()" << endl;

	// here we need to retrieved saved bookmarks
}

void BookmarksPart::projectClosed()
{
	kdDebug(0) << "BookmarksPart::projectClosed()" << endl;

	// here we need to save bookmarks
}


EditorData * BookmarksPart::storeBookmarksForURL( KParts::ReadOnlyPart * ro_part )
{
	if ( KTextEditor::MarkInterface * mi = dynamic_cast<KTextEditor::MarkInterface *>(ro_part) )
	{
		EditorData * data = new EditorData;
		data->url = ro_part->url();

		// removing previous data for this url, if any
		if ( _editorMap.remove( data->url.path() ) )
		{
			kdDebug(0) << "removed previous data" << endl;
		}

		QPtrList<KTextEditor::Mark> marks = mi->marks();
		QPtrListIterator<KTextEditor::Mark> it( marks );
		while ( it.current() )
		{
			if ( it.current()->type == KTextEditor::MarkInterface::markType01 )
			{
				kdDebug(0) << "Found bookmark. Line: " << it.current()->line << endl;
				data->marks.append( it.current()->line );
			}
			++it;
		}

		if ( ! data->marks.isEmpty() )
		{
			kdDebug(0) << data->marks.count() << " bookmarks in " << data->url.path() << " - Keeping" << endl;

			_editorMap.insert( data->url.path(), data );
		}
		else
		{
			kdDebug(0) << "No bookmarks in " << data->url.path() << " - Deleting" << endl;
			delete data;
			data = 0;
		}
		kdDebug(0) << "_editorMap.count(): " << _editorMap.count() << endl;
		return data;
	}
	return 0;
}

void BookmarksPart::examineLoadedParts()
{
	if( const QPtrList<KParts::Part> * partlist = partController()->parts() )
	{
		QPtrListIterator<KParts::Part> it( *partlist );
		while ( KParts::Part* part = it.current() )
		{
			if ( KParts::ReadOnlyPart * ro_part = dynamic_cast<KParts::ReadOnlyPart *>( part ) )
			{
				storeBookmarksForURL( ro_part );
			}
			++it;
		}
	}
}



#include "bookmarks_part.moc"
