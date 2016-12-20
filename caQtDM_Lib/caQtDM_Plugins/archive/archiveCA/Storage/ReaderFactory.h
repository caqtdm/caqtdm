// -*- c++ -*-

#ifndef __READER_FACTORY_H__
#define __READER_FACTORY_H__

// Tools
#include <ToolsConfig.h>
// Storage
#include <Index.h>
#include <DataReader.h>

/// \addtogroup Storage
/// @{

/// Create one of the DataReader class instances.
class ReaderFactory
{
public:
    /// Determine what DataReader to use:
    enum How
    {
        Raw,     ///< Use RawDataReader
        Plotbin, ///< Use PlotReader
        Average, ///< Use AverageReader
        Linear   ///< Use LinearReader
    };

    /// String representation of how/delta.
    ///
    /// The result is suitable for display ala
    /// "Method: ...".
    /// The result is held in a static char buffer,
    /// beware of other threads calling toString.
    static const char *toString(How how, double delta);
    
    /// Create a DataReader.
    static DataReader *create(Index &index, How how, double delta);
};

/// @}

#endif
