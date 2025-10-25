#include <string>
#include <vector>
#include <iostream>
#include "../flow-handler/control_flow_handler.h"

namespace FileReader {
    
    // A struct representing a stream of characters from an opened file.
    struct FileStream {
        /* The source directory of this file stream.*/
        std::string fileName;
        /* The raw stream of characters from this file.*/
        std::string stream;
    };

    class SourceFilesManager {
        
        std::vector<FileStream> fileStreams;

        public:
        SourceFilesManager();
        ~SourceFilesManager();

        void ReadSourceFile(std::string fileName, ControlFlow::ControlFlowHandler& fh);
    };
    
}