#pragma once
#include <string>
#include <vector>
#include "../file-reading/file_reader.h"

/*
Control flow and error handling for compilation.

Calling functions should call `NewStep()` before calling a function or
beggining a procedure, and the callee is responsible for calling 
`CompleteStep()` at its end. 
*/
namespace ControlFlow {

    // Error codes for compilation (and runtime?) errors 
    constexpr inline int ERRCODE_UNKNOWN_TOKEN = 0;
    constexpr inline int ERRCODE_UNREADABLE_FILE = 1;
    constexpr inline int ERRCODE_SYNTAX_ERROR = 2;
    
    // status codes for compilation (and runtime?) results. Tells the flow handler what to do next 
    constexpr inline int STATUSCODE_NOT_FINNISHED    = -1;
    constexpr inline int STATUSCODE_SUCCESS_CONTINUE =  0;
    constexpr inline int STATUSCODE_WARNING_CONTINUE =  1;
    constexpr inline int STATUSCODE_ERROR_EXIT       =  2;
    constexpr inline int STATUSCODE_ERROR_CONTINUE   =  3;
    
    // other constants
    /* signals that no next or substep exists.*/
    constexpr inline int STEP_NULL = -1;
    
    enum class CompilationErrorSeverity {WARNING, ERROR};

    struct CompilationError {
        std::string errorMessage;
        int errorCode;
        CompilationErrorSeverity severity;
        FileReader::SourceString sourceString; // the part of the code where the compilation error occured. `sourceString.IsNull()` will be true if there was no specific part of source code associated with this error.

        CompilationError(CompilationErrorSeverity status, unsigned int errorCode, std::string errorMessage, FileReader::SourceString sourceString);
    };
    
    struct CompilationStepResult {
        int statusCode;

        bool operator==(const CompilationStepResult& r) const;
    };

    const inline CompilationStepResult COMPILATION_STEP_NOT_FINNISHED = {
        .statusCode = STATUSCODE_NOT_FINNISHED
    };

    struct CompilationStep {
        std::vector<CompilationError> errors;
        /* The result of this step. `NULL` if the step is not ye*/
        CompilationStepResult result = COMPILATION_STEP_NOT_FINNISHED;

        /* index of the next step in the steps vector. -1 if there is no*/
        int nextStepIndex = STEP_NULL; 
        int subStepIndex = STEP_NULL;
    };

    class ControlFlowHandler {
        private:
        std::vector<CompilationStep> compilationSteps;
        /* When traversing the step stack down, this vector stores the indexes of the parents.*/
        std::vector<int> parentIndexStepStack;
        int currentStepIndex = 0;

        public:
        ControlFlowHandler();
        ~ControlFlowHandler();

        CompilationError* Error(CompilationErrorSeverity severity, unsigned int errorCode, std::string errorMessage, const FileReader::SourceString& sourceString); 
        void NewStep(bool down = false);
        void CompleteStep(int statusCode = STATUSCODE_SUCCESS_CONTINUE, bool up = false);  // TODO: replace up down next with enums

        void Compile(const char* filename);
    };
}