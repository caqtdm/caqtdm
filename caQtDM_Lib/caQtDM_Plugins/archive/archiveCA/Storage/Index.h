/// -*- c++ -*-

#ifndef __INDEX_H__
#define __INDEX_H__

// Tools
#include <ToolsConfig.h>
// Storage
#include <NameHash.h>

/// \addtogroup Storage
/// \@{

/// Base class for the archiver's indices.
class Index
{
public:
    Index() {}

    virtual ~Index() {}

    /// Open an index.
    ///
    /// @exception GenericException on error
    ///            (file not found, wrong file format, ...).
    virtual bool open(const stdString &filename, char *buff, bool readonly=true) = 0;

    /// Close the index.
    virtual void close() = 0;
    
    /// Add a channel to the index.
    ///
    /// A channel has to be added before data blocks get defined
    /// for the channel. When channel is already in index, existing
    /// tree gets returned.
    ///
    /// @return RTree pointer which caller must delete.
    /// @exception GenericException on internal error.
    virtual class RTree *addChannel(const stdString &channel,
                                    stdString &directory) = 0;

    /// Obtain the RTree for a channel.
    ///
    /// Directory is set to the path/directory of the index,
    /// which together with the data block in the RTree will then
    /// lead to the actual data files.
    ///
    /// @return RTree pointer which caller must delete.
    ///         Returns 0 if the channel is not found.
    /// @exception GenericException on internal error.
    virtual class RTree *getTree(const stdString &channel,
                                 stdString &directory) = 0;
    
    /// Used by get_first_channel(), get_next_channel().
    class NameIterator
    {
    public:
        const stdString &getName() 
        {    return entry.name; }
    private:
        friend class IndexFile;
        friend class ListIndex;
        uint32_t hashvalue;
        NameHash::Entry entry;
    };

    /// Locate NameIterator on first channel.
    ///
    /// Actually, concurrent iteration is not supported.
    /// So one can use only one NameIterator at a time.
    ///
    /// @return true if there was a first entry.
    virtual bool getFirstChannel(NameIterator &iter) = 0;

    /// Locate NameIterator on next channel.
    ///
    /// @pre Successfull call to get_first_channel().
    /// @return true if there was another entry.
    virtual bool getNextChannel(NameIterator &iter) = 0;
protected:
    PROHIBIT_DEFAULT_COPY(Index);
};

/// \@}

#endif
