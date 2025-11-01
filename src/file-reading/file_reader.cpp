#include "file_reader.h"

namespace FileReader {

    FileStream::FileStream(std::string fileName, size_t fileSize, size_t fileIndex)
        : stream(fileSize, '\0') 
    {
        this->fileName = fileName;
        this->fileIndex = fileIndex;
    }

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

        
        fseek(file, 0, SEEK_END);
        size_t fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);
        
        // add an entry to the `fileStreams` vector
        fileStreamIndexMap[filename] = fileStreams.size();
        fileStreams.emplace_back(filename, fileSize, fileStreams.size());
        
        std::fread(&fileStreams.back().stream[0], 1, fileSize, file);

        std::cout << "read " << fileStreams.back().stream << " from the file.\n";

        std::fclose(file);

        fh.CompleteStep(ControlFlow::STATUSCODE_SUCCESS_CONTINUE, false);
    }

    FileStream* SourceFilesManager::GetFileStream(std::string fileName) {

        auto fileIndexIter = fileStreamIndexMap.find(fileName);
        if (fileIndexIter == fileStreamIndexMap.end()) {
            throw std::logic_error("A file with that name has not been added to the file streams list.");
            return nullptr;
        }
        return &fileStreams[fileIndexIter->second];
    }
    FileStream* SourceFilesManager::GetTopFileStream() {
        return &fileStreams.back();
    }
}