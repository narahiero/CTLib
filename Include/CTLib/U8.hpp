//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#pragma once

/*! @file U8.hpp
 * 
 *  @brief The header the U8 module.
 */


#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include <CTLib/Memory.hpp>
#include <CTLib/Utilities.hpp>


namespace CTLib
{

class U8Arc;
class U8Dir;
class U8File;

/*! @defgroup u8 U8
 *  
 *  @addtogroup u8
 * 
 *  @brief The U8 module contains all classes and methods that can be used to
 *  interact with Nintendo's U8 archive file format.
 *  @{
 */

/*! @brief Enumeration of U8 entry types. */
enum class U8EntryType
{
    /*! @brief The U8 entry type for a directory. */
    Directory,
    
    /*! @brief The U8 entry type for a file. */
    File
};

/*! @brief Super-class of U8Dir and U8File. */
class U8Entry
{

    friend class U8Arc;

public:

    virtual ~U8Entry();

    /*! @brief Renames this entry with the specified name.
     *  
     *  @param[in] name The new name of this entry
     * 
     *  @throw CTLib::U8Error If an entry in the same directory as this has the
     *  specified name.
     */
    void rename(const std::string& name);

    /*! @brief Returns the directory containing this entry. */
    U8Dir* getParent() const;

    /*! @brief Returns the name of this entry. */
    std::string getName() const;

    /*! @brief Returns the type of this entry. */
    virtual U8EntryType getType() const = 0;

    /*! @brief Returns `this` as a U8File pointer.
     *  
     *  Returns `this` as a U8File pointer if, and only if,
     *  @link CTLib::U8Entry::getType() getType()@endlink returns the value
     *  CTLib::U8EntryType::File. If not a U8File throws a U8Error.
     * 
     *  @throw CTLib::U8Error If `this` is not a U8 file.
     * 
     *  @return `this`
     */
    virtual U8File* asFile() const = 0;

    /*! @brief Returns `this` as a U8Dir pointer.
     *  
     *  Returns `this` as a U8Dir pointer if, and only if,
     *  @link CTLib::U8Entry::getType() getType()@endlink returns the value
     *  CTLib::U8EntryType::Directory. If not a U8Dir throws a U8Error.
     * 
     *  @throw CTLib::U8Error If `this` is not a U8 directory.
     * 
     *  @return `this`
     */
    virtual U8Dir* asDirectory() const = 0;

protected:

    //! creates entry and adds this entry to the specified archive
    U8Entry(U8Arc* arc);

    //! creates entry and adds this entry to the specified archive and dir
    U8Entry(U8Arc* arc, U8Dir* parent, const std::string& name);

    //! the archive owning this entry
    U8Arc* arc;

    //! the parent directory
    U8Dir* parent;

    //! name of this entry
    std::string name;

private:

    U8Entry(const U8Entry&) = delete;

    U8Entry(U8Entry&&) = delete;

    // throws if the specified name is invalid
    void assertValidName(const std::string& name) const;
};

/*! @brief A directory within a U8 archive. */
class U8Dir final : public U8Entry
{

    friend class U8Arc;
    friend class U8Entry;

public:

    /*! @brief An iterator to iterate over the entries in this directory. */
    using Iterator = MapValueIterator<std::string, U8Entry*>;

    ~U8Dir();

    /*! @brief Adds a directory with the specified name.
     * 
     *  @param[in] name The directory name
     * 
     *  @throw CTLib::U8Error If an entry with the specified name already
     *  exists.
     * 
     *  @return The newly created directory
     */
    U8Dir* addDirectory(const std::string& name);

    /*! @brief Adds a file with the specified name.
     * 
     *  @param[in] name The file name
     * 
     *  @throw CTLib::U8Error If an entry with the specified name already
     *  exists.
     * 
     *  @return The newly created file
     */
    U8File* addFile(const std::string& name);

    /*! @brief Returns the number of entries in this directory, not recursive.
     */
    size_t count() const;

    /*! @brief Returns the entry with the specified name, or `nullptr` if no
     *  entry with such name exists.
     *  
     *  @param[in] name The name of the entry
     * 
     *  @return The entry with the specified name, or `nullptr`.
     */
    U8Entry* getEntry(const std::string& name) const;

    /*! @brief Returns whether this directory has an entry with the specified
     *  name.
     */
    bool hasEntry(const std::string& name) const;

    /*! @brief Returns CTLib::U8EntryType::Directory. */
    U8EntryType getType() const override;

    /*! @brief Throws CTLib::U8Error. */
    U8File* asFile() const override;

    /*! @brief Returns `this`. */
    U8Dir* asDirectory() const override;

    /*! @brief Returns an iterator pointing to the first entry. */
    Iterator begin();

    /*! @brief Returns an iterator pointing after the last entry. */
    Iterator end();

private:

    // simply calls matching U8Entry constructor
    U8Dir(U8Arc* arc);

    // simply calls matching U8Entry constructor
    U8Dir(U8Arc* arc, U8Dir* parent, const std::string& name);

    // throws if an entry with the specified name already exists
    void assertUniqueName(const std::string& name);

    // map of <name, entry> containing all entries in this directory
    std::map<std::string, U8Entry*> entries;
};

/*! @brief A file within a U8 archive. */
class U8File final : public U8Entry
{

    friend class U8Dir;

public:

    ~U8File();

    /*! @brief Returns CTLib::U8EntryType::File. */
    U8EntryType getType() const override;

    /*! @brief Returns `this`. */
    U8File* asFile() const override;

    /*! @brief Throws CTLib::U8Error. */
    U8Dir* asDirectory() const override;

private:

    // simply calls matching U8Entry constructor
    U8File(U8Arc* arc, U8Dir* parent, const std::string& name);
};

/*! @brief The U8Arc class is a object-oriented representation of Nintendo's U8
 *  archive file format.
 */
class U8Arc final
{

    friend class U8Entry;

public:

    /*! @brief An iterator to iterate over the entries in this archive. */
    using Iterator = std::vector<U8Entry*>::iterator;

    /*! @brief Constructs an empty U8 archive. */
    U8Arc();

    /*! @brief Delete copy constructor for move-only class. */
    U8Arc(const U8Arc&) = delete;

    /*! @brief Moves the contents of the specified U8 archive in this newly
     *  created one.
     * 
     *  The source archive will then be empty, but can still be safely used.
     * 
     *  @param[in] src The archive to move
     */
    U8Arc(U8Arc&& src);

    ~U8Arc();

    /*! @brief Adds a directory with the specified name.
     * 
     *  @param[in] name The directory name
     * 
     *  @throw CTLib::U8Error If an entry with the specified name already
     *  exists.
     * 
     *  @return The newly created directory
     */
    U8Dir* addDirectory(const std::string& name);

    /*! @brief Adds a file with the specified name.
     * 
     *  @param[in] name The file name
     * 
     *  @throw CTLib::U8Error If an entry with the specified name already
     *  exists.
     * 
     *  @return The newly created file
     */
    U8File* addFile(const std::string& name);

    /*! @brief Adds a directory at the specified _forward slash separated_
     *  absolute path.
     *
     *  Any missing parent directory will be created.
     * 
     *  If any of the parent directories in the specified path designate an
     *  already existing file, a U8Error is thrown.
     *  
     *  @param[in] path The absolute path of the directory to be created
     * 
     *  @throw CTLib::U8Error If an entry at the specified path already exists,
     *  or if any parent directory in `path` designate an already existing
     *  file.
     * 
     *  @return The newly created directory
     */
    U8Dir* addDirectoryAbsolute(const std::string& path);

    /*! @brief Adds a file at the specified _forward slash separated_ absolute
     *  path.
     *  
     *  Any missing parent directory will be created.
     * 
     *  If any of the parent directories in the specified path designate an
     *  already existing file, a U8Error is thrown.
     *  
     *  @param[in] path The absolute path of the file to be created
     * 
     *  @throw CTLib::U8Error If an entry at the specified path already exists,
     *  or if any parent directory in `path` designate an already existing
     *  file.
     * 
     *  @return The newly created file
     */
    U8File* addFileAbsolute(const std::string& path);

    /*! @brief Returns the number of entries in the root of this archive. */
    size_t count() const;

    /*! @brief Returns the number of entries in this entire archive. */
    size_t totalCount() const;

    /*! @brief Returns the entry with the specified name, or `nullptr` if no
     *  entry with such name exists.
     *  
     *  @param[in] name The name of the entry
     * 
     *  @return The entry with the specified name, or `nullptr`
     */
    U8Entry* getEntry(const std::string& name) const;

    /*! @brief Returns whether there is an entry with the specified name in the
     *  root of this archive.
     */
    bool hasEntry(const std::string& name) const;

    /*! @brief Returns the entry designated by the specified _forward slash_
     *  _delimited_ absolute path.
     *
     *  Consider the following code:
     *  ~~~{.cpp}
     *  U8Arc arc;
     *  arc.addDirectory(".")->addDirectory("posteffect")->addFile("posteffect.blight");
     *  // ...
     *  U8Entry* file = arc.getAbsoluteEntry("./posteffect/posteffect.blight");
     *  ~~~
     * 
     *  As you can see at the end of this code snippet, the nested BLIGHT file
     *  is accessed directly from the archive using its absolute path.
     * 
     *  If no entry is found at the specified path, `nullptr` is returned.
     * 
     *  @param[in] path The absolute path to the entry
     * 
     *  @return The entry designated by the specified path, or `nullptr`
     */
    U8Entry* getEntryAbsolute(const std::string& path) const;

    /*! @brief Returns whether an entry is found at the specified path.
     *  
     *  @see CTLib::U8Arc::getEntryAbsolute() for more information
     */
    bool hasEntryAbsolute(const std::string& path) const;

    /*! @brief Returns an iterator pointing to the first entry. */
    Iterator begin();

    /*! @brief Returns an iterator pointing after the last entry. */
    Iterator end();

private:

    // adds an entry of the specified type at the specified path
    U8Entry* addEntryAbsolute(const std::string& path, U8EntryType type);

    // vector containing all entries in this archive
    std::vector<U8Entry*> entries;

    // unnamed directory containing all entries
    U8Dir* root;
};

/*! @brief U8Error is the error class used by the methods in this header. */
class U8Error final : std::runtime_error
{

public:

    /*! @brief Constructs a CTLib::U8Error with the specified error message. */
    U8Error(const char* msg);

    /*! @brief Constructs a CTLib::U8Error with the specified error message. */
    U8Error(const std::string& msg);
};

/*! @} addtogroup u8 */
}
