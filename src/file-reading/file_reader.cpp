#include "file_reader.h"
#include "../flow-handler/control_flow_handler.h"

namespace FileReader {

    SourceFilesManager sourceFilesManager;

    FileStream::FileStream(std::string fileName, size_t fileSize, size_t fileIndex)
        : stream(fileSize, '\0') 
    {
        this->fileName = fileName;
        this->fileIndex = fileIndex;
    }

    // for now, leave empty
    SourceFilesManager::SourceFilesManager() {}
    SourceFilesManager::~SourceFilesManager() {}

    bool SourceString::IsNull() const {
        return fileIndex == SOURCESTRING_NULL_INDICATOR;
    }

    SourceString::SourceString() {
        fileIndex = SOURCESTRING_NULL_INDICATOR;
        startIndex = 0;
        length = 0;
        lineNumber = 0;
    }

    SourceString::SourceString(int fileIndex, size_t lineNumber, size_t startIndex, size_t length) {
        this->fileIndex = fileIndex;
        this->lineNumber = lineNumber;
        this->startIndex = startIndex;
        this->length = length;
    }

    std::string SourceString::FileName() const {
        return sourceFilesManager.GetFileStream(fileIndex)->fileName;
    }

    std::string_view SourceString::GetString(bool entireLine) const {
        std::string_view view(sourceFilesManager.GetFileStream(fileIndex)->stream);

        if (entireLine) {
            size_t l = view.rfind('\n', startIndex);
            size_t r = view.find('\n', startIndex + length);
            if (l == std::string::npos) {l = 0;} else {l+=1;}
            if (r == std::string::npos) {r = view.length() - 1;} else {r-=1;}
            return view.substr(l, r-l+1);
        } else {
            return view.substr(startIndex, length);
        }
    }

    std::string SourceString::GetUnderlineString() const {
        std::string_view view(sourceFilesManager.GetFileStream(fileIndex)->stream);
        size_t l = view.rfind('\n', startIndex);
        size_t r = view.find('\n', startIndex + length);
        if (l == std::string::npos) {l = 0;} else {l+=1;}
        if (r == std::string::npos) {r = view.length() - 1;} else {r-=1;}

        // construct the underline string
        std::string s(r-l+1, ' ');
        for (size_t i = startIndex - l; i < startIndex - l + length; i++) {
            s[i] = '^';
        }
        return s;
    }
    
    void SourceFilesManager::ReadSourceFile(std::string filename, ControlFlow::ControlFlowHandler& fh) {

        std::FILE* file = std::fopen(filename.data(),"r");

        if (file == nullptr) {

            std::string errorMessage = "Could not read the file content of ";
            errorMessage.append(filename);
            fh.Error(
                ControlFlow::CompilationErrorSeverity::ERROR, 
                ControlFlow::ERRCODE_UNREADABLE_FILE,
                errorMessage,
                SourceString()
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

    FileStream* SourceFilesManager::GetFileStream(size_t fileIndex) {

        if (fileIndex >= fileStreams.size()) {
            throw std::logic_error("The fileIndex was out of range!");
            return nullptr;
        }
        return &fileStreams[fileIndex];
    }

    FileStream* SourceFilesManager::GetTopFileStream() {
        return &fileStreams.back();
    }
}