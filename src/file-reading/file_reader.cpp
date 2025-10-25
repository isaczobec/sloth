#include "file_reader.h"

namespace FileReader {

    // for now, leave empty
    SourceFilesManager::SourceFilesManager() {}
    SourceFilesManager::~SourceFilesManager() {}
    
    void SourceFilesManager::ReadSourceFile(std::string filename, ControlFlow::ControlFlowHandler& fh) {

        std::FILE* file = std::fopen(filename.data(),"r");

        if (file == nullptr) {

            std::string errorMessage = "Could not read the file content of ";
            errorMessage.append(filename);
            fh.Error(
                ControlFlow::CompilationErrorSeverity::ERROR, 
                ControlFlow::ERRCODE_UNREADABLE_FILE,
                errorMessage
            );
            return;
        }

        int c;
        
        fseek(file, 0, SEEK_END);
        size_t fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);
        
        std::string s(fileSize, '\0');
        std::fread(&s[0], 1, fileSize, file);

        std::cout << "read " << s << " from the file.";

        std::fclose(file);
    }
}