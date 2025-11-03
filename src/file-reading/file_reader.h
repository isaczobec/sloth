#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <unordered_map>

// Forward declaration of ControlFlowHandler
namespace ControlFlow {
    class ControlFlowHandler;
}

namespace FileReader {
    
    class SourceFilesManager;
    // the singleton instance of `SourceFilesManager`.
    extern SourceFilesManager sourceFilesManager;

    // A struct representing a stream of characters from an opened file.
    struct FileStream {
        /* The source directory of this file stream.*/
        std::string fileName;
        /* The raw stream of characters from this file.*/
        std::string stream;

        size_t fileIndex;

        FileStream(std::string fileName, size_t fileSize, size_t fileIndex);
    };

    // used to indicate that a `SourceString` is pointing to a null value.
    constexpr inline int SOURCESTRING_NULL_INDICATOR = -1;

    // A struct pointing to a range of characters within a source file
    struct SourceString {
        int fileIndex;
        size_t lineNumber;
        size_t startIndex; // which character index into the filestream the sourcestring starts at.
        size_t length;

        SourceString(); // Parameterless constructor. Sets the sourcestring to point to null.
        SourceString(int fileIndex, size_t lineNumber, size_t startIndex, size_t length);
        bool IsNull() const; // returns `true` if `SourceString` is pointing to no source code.
        std::string FileName() const;

        // Gets a view of the source code this `SourceString` is pointing to. If `entireLine` is `true`, then the entire line or lines of this string is returned.
        std::string_view GetString(bool entireLine = true) const;
        std::string GetUnderlineString() const; // constructs a string that underlines the line given by `GetString(bool entireLine = true)`.
    };

    class SourceFilesManager {
        
        std::vector<FileStream> fileStreams;
        /* A map of indicies into fileStreams for fast lookups.*/
        std::unordered_map<std::string, size_t> fileStreamIndexMap; 

        public:
        SourceFilesManager();
        ~SourceFilesManager();

        void ReadSourceFile(std::string fileName, ControlFlow::ControlFlowHandler& fh); // Reads the contents of `fileName` and emplaces a new `FileStream` at the back of `fileStreams`.
        FileStream* GetTopFileStream();
        FileStream* GetFileStream(std::string fileName);
        FileStream* GetFileStream(size_t fileIndex);

    };

    
}