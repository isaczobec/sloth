#include <string>
#include <vector>
#include <iostream>
#include "../flow-handler/control_flow_handler.h"
#include <unordered_map>

namespace FileReader {
    
    // A struct representing a stream of characters from an opened file.
    struct FileStream {
        /* The source directory of this file stream.*/
        std::string fileName;
        /* The raw stream of characters from this file.*/
        std::string stream;

        size_t fileIndex;

        FileStream(std::string fileName, size_t fileSize, size_t fileIndex);
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

    };
    
}