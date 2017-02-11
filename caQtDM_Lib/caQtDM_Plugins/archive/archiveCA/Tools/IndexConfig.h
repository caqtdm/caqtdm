// -*- c++ -*-

#ifndef __INDEX_CONFIG_H__
#define __INDEX_CONFIG_H__

// Tools
#include <ToolsConfig.h>

/// \ingroup Tools

/// Parser for indexconfig.dtd.

/// The index configuration is used by both the ArchiveIndexTool
/// and the simple file-by-file mechanism supported by the
/// ListedIndex.
class IndexConfig
{
public:
    /// Parse the given configuration file.
    ///
    /// Since the ListIndex might use this config file
    /// as well as a "real" binary index file,
    /// some care is taken in here to assert that
    /// we're really dealing with a config. file and not
    /// a binary index file.
    ///
    /// @return true if the file exists and parses OK.
    /// false if the file exists but is no XML file,
    /// so it can't possibly be an index config.
    /// @exception Throws GenericException in case
    /// the file cannot be opened at all or there
    /// is an XML error while parsing it as an indexconfig.
    bool parse(const stdString &config_name);
    
    /// List of sub-archives found in the indexconfig file.
    ///
    /// Only valid after parse succeeds.
    stdList<stdString> subarchives;
};

#endif

