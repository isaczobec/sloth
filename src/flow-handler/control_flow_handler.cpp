#include "control_flow_handler.h"
#include "../file-reading/file_reader.h"
#include "../tokenization/tokenizer.h"
#include "../syntax-parsing/syntax_rules.h"

#include <iostream>
#include <string>
#include <regex>
#include <iterator>
#include <cstdio>
#include <vector>
#include <stdexcept>

namespace ControlFlow {

    CompilationError::CompilationError(CompilationErrorSeverity severity, unsigned int errorCode, std::string errorMessage, FileReader::SourceString sourceString) 
        : errorMessage(errorMessage), severity(severity), errorCode(errorCode), sourceString(sourceString) {}

    bool CompilationStepResult::operator==(const CompilationStepResult& r) const {
        return r.statusCode == statusCode;
    }

    ControlFlowHandler::ControlFlowHandler() 
    {
        // push a default step to the `compilationSteps` since `NetStep()` assumes it is not empty
        compilationSteps.emplace_back();
        CompleteStep(); // the default step is "completed" instantly
    }

    ControlFlowHandler::~ControlFlowHandler() {} // TODO: no destructor for now

    void ControlFlowHandler::Error(CompilationErrorSeverity severity, unsigned int errorCode, std::string errorMessage, const FileReader::SourceString& sourceString) {
        compilationSteps.back().errors.emplace_back(severity, errorCode, errorMessage, sourceString);

        // construct an error string
        std::string errorString;
        if      (severity == CompilationErrorSeverity::ERROR) {errorString += "Error (";}
        else if (severity == CompilationErrorSeverity::WARNING) {errorString += "Warning (";}
        errorString.append(std::to_string(errorCode));
        errorString += "): ";
        errorString += errorMessage;

        if (!sourceString.IsNull()) {
            errorString += "\nat ";
            errorString += std::to_string(sourceString.lineNumber);
            errorString += " in ";
            errorString.append(sourceString.FileName());
            errorString += "\n";
            errorString.append(sourceString.GetString(true));
            errorString += "\n";
            errorString.append(sourceString.GetUnderlineString());
        }

        std::cout << errorString << std::endl;
    }

    void ControlFlowHandler::NewStep(bool down) {

        size_t prevStepIndex = currentStepIndex;
        
        if (down) 
        {
            if (compilationSteps[prevStepIndex].subStepIndex != STEP_NULL) {
                throw std::logic_error("Tried to insert a substep into a compilation step that already has a substep. returning.");
                return;
            }
            
            // push the previous step index to the parent index stack 
            parentIndexStepStack.push_back(currentStepIndex);
            compilationSteps.emplace_back(); // construct the new step
            currentStepIndex = compilationSteps.size() - 1;
            compilationSteps[prevStepIndex].subStepIndex = currentStepIndex;
        } 
        else 
        {
            // throw an error if the previous step has not yet been completed
            if (compilationSteps[prevStepIndex].result == COMPILATION_STEP_NOT_FINNISHED) {
                throw std::logic_error("Tried to insert a next step while the current one has not yet been finished. Call ControlFlowHandler::CompleteStep() before beginning a new step.");
                return;
            }

            compilationSteps.emplace_back(); // construct the new step
            currentStepIndex = compilationSteps.size() - 1;
            compilationSteps[prevStepIndex].nextStepIndex = currentStepIndex;
        }
    }

    void ControlFlowHandler::CompleteStep(int statusCode, bool up) {
        
        compilationSteps[currentStepIndex].result.statusCode = statusCode;
        
        if (up) {
            currentStepIndex = parentIndexStepStack.back(); 
            parentIndexStepStack.pop_back(); // TODO: Add stack underflow checking
        }
    }

    void ControlFlowHandler::Compile(const char* filename) {
        using namespace ParseTree;

        // File reading
        NewStep(); 
        FileReader::sourceFilesManager.ReadSourceFile(std::string(filename), *this);
        
        // Tokenization
        Tokenization::Tokenizer t;
        NewStep();
        t.Tokenize(FileReader::sourceFilesManager.GetTopFileStream(), *this);

        // Parse tree creation
        ParseTreeBuilder builder;
        int tokenPtr = 0;
        ParseTreeNode* node = builder.ParseNode(&Rules::STATEMENT, t.GetTokens(), tokenPtr, *this);

        std::cout << "done" << std::endl;

    }
}
