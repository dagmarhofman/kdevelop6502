/* This file is part of KDevelop
    Copyright 2006 Hamish Rodda <rodda@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef TOPDUCONTEXT_H
#define TOPDUCONTEXT_H

#include "ducontext.h"
#include "languageexport.h"
#include <QtCore/QMutex>

template< class T >
class KSharedPtr;

namespace KDevelop
{
  class QualifiedIdentifier;
  class DUChain;
  class IdentifiedFile; //Defined in parsingenvironment.h
  class ParsingEnvironmentFile;

/**
 * The top context in a definition-use chain for one source file.
 *
 * Implements SymbolTable lookups and locking for the chain.
 *
 * \todo move the registration with DUChain here
 */
class KDEVPLATFORMLANGUAGE_EXPORT TopDUContext : public DUContext
{
public:
  explicit TopDUContext(KTextEditor::Range* range, ParsingEnvironmentFile* file = 0);
  virtual ~TopDUContext();

  TopDUContext* topContext() const;

  /**
   * There may be multiple context's for one file, but each of those should have a different identity().
   *  */
  IdentifiedFile identity() const;

  /**
   * @see ParsingEnvironmentFile
   * May return zero if no file was set.
   * */
  KSharedPtr<ParsingEnvironmentFile> parsingEnvironmentFile() const;
  
  /// Returns true if this object is being deleted, otherwise false.
  bool deleting() const;

  bool hasUses() const;
  void setHasUses(bool hasUses);

  /// Returns true if this object is registered in the du-chain. If it is not, all sub-objects(context, declarations, etc.)
  bool inDuChain() const;
  /// This flag is only used by DUChain, never change it from outside.
  void setInDuChain(bool);
  
  /**
   * Determine if this chain imports another chain.
   *
   * \note you must be holding a read but not a write chain lock when you access this function.
   */
  virtual bool imports(const DUContext* origin, const KTextEditor::Cursor& position) const;

  /**
   * Returns the trace of imports from this context top the given target.
   * The positions in the returned trace may be invalid.
   * */
  DUContext::ImportTrace importTrace(const TopDUContext* target) const;

  enum Flags {
    NoFlags = 0,
    /**
     * A language-specific flag used by C++ to mark one context as a proxy of another.
     * If this flag is set on a context, the first imported context should be used for any computations
     * like searches, listing, etc. instead of using this context.
     *
     * Note: This flag does not directly change the behavior of the language-independent du-chain.
     * */
    ProxyContextFlag = 1,
    /**
     * Placeholder used in searching-functions, when the flag should simply be ignored.
     * */
    AnyFlag=2,
    LastFlag
  };
  
  /**
   * Use flags to mark top-contexts for special behavior. Any flags above LastFlag may be used for language-specific stuff.
   * */
  Flags flags() const;
  void setFlags(Flags f);

  virtual void addImportedParentContext(DUContext* context, const KTextEditor::Cursor& position = KTextEditor::Cursor(), bool anonymous=false);
  virtual void removeImportedParentContext(DUContext* context);
  
protected:
  void setParsingEnvironmentFile(ParsingEnvironmentFile*) const;
  
  virtual bool findDeclarationsInternal(const QList<QualifiedIdentifier>& identifiers, const KTextEditor::Cursor& position, const AbstractType::Ptr& dataType, QList<Declaration*>& ret, const ImportTrace& trace, bool inImportedContext) const;

  /// Return those \a declarations that are visible in this document from \a position and are of the specified \a dataType
  QList<Declaration*> checkDeclarations(const QList<Declaration*>& declarations, const KTextEditor::Cursor& position, const AbstractType::Ptr& dataType) const;

  virtual void findContextsInternal(ContextType contextType, const QList<QualifiedIdentifier>& identifier, const KTextEditor::Cursor& position, QList<DUContext*>& ret, SearchFlags flags = NoSearchFlags) const;

  /// Place \a contexts of type \a contextType that are visible in this document from \a position in a \a{ret}urn list
  void checkContexts(ContextType contextType, const QList<DUContext*>& contexts, const KTextEditor::Cursor& position, QList<DUContext*>& ret) const;

  /**
   * Does the same as DUContext::updateAliases, except that it uses the symbol-store, and processes the whole identifier.
   * @param canBeNamespace whether the searched identifier may be a namespace.
   * If this is true, namespace-aliasing is applied to the last elements of the identifiers.
   * */
  void applyAliases( const QList<QualifiedIdentifier>& identifiers, QList<QualifiedIdentifier>& target, const KTextEditor::Cursor& position, bool canBeNamespace, int startPos = 0, int maxPos = -1 ) const;
  
private:
  class TopDUContextPrivate* const d;
  friend class TopDUContextPrivate;
  friend class DUChain; //To allow access to setParsingEnvironmentFile
};

}

#endif // TOPDUCONTEXT_H

// kate: space-indent on; indent-width 2; tab-width 4; replace-tabs on; auto-insert-doxygen on
