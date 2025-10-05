#include "control_flow_handler.h"
#include "tokenizer.h"
#include <iostream>
#include <string>
#include <regex>
#include <iterator>
#include <cstdio>
#include <vector>
#include <stdexcept>

namespace ControlFlow {

    CompilationError::CompilationError(CompilationErrorSeverity severity, unsigned int errorCode, std::string errorMessage) 
        : errorMessage(errorMessage), severity(severity), errorCode(errorCode) {}

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

    void ControlFlowHandler::Error(CompilationErrorSeverity severity, unsigned int errorCode, std::string errorMessage) {
        compilationSteps.back().errors.emplace_back(severity, errorCode, errorMessage);
        std::cout << "ERR" << errorCode << ": " << errorMessage << std::endl;
    }

    void ControlFlowHandler::NewStep(bool down) {

        CompilationStep prevStep = compilationSteps[currentStepIndex];

        
        if (down) 
        {
            if (prevStep.subStepIndex != STEP_NULL) {
                throw std::logic_error("Tried to insert a substep into a compilation step that already has a substep. returning.");
                return;
            }
            
            // push the previous step index to the parent index stack 
            parentIndexStepStack.push_back(currentStepIndex);
            compilationSteps.emplace_back(); // construct the new step
            prevStep.subStepIndex = currentStepIndex;
            currentStepIndex = compilationSteps.size() - 1;
        } 
        else 
        {
            // throw an error if the previous step has not yet been completed
            if (prevStep.result == COMPILATION_STEP_NOT_FINNISHED) {
                throw std::logic_error("Tried to insert a next step while the current one has not yet been finished. Call ControlFlowHandler::CompleteStep() before beginning a new step.");
                return;
            }

            compilationSteps.emplace_back(); // construct the new step
            prevStep.nextStepIndex = currentStepIndex;
            currentStepIndex = compilationSteps.size() - 1;
        }
    }

    void ControlFlowHandler::CompleteStep(int statusCode, bool up) {
        
        compilationSteps[currentStepIndex].result.statusCode = statusCode;
        
        if (up) {
            currentStepIndex = parentIndexStepStack.back(); 
            parentIndexStepStack.pop_back(); // TODO: Add stack underflow checking
        }
    }

    void ControlFlowHandler::Compile() {

        std::FILE* file = std::fopen("test","r");

        std::string s;
        int c;
        while ((c = std::fgetc(file)) != EOF) {
            s.append(1, (char)c);
        }
        
        Tokenization::Tokenizer t;

        NewStep();
        t.Tokenize(s, *this);

        for (Tokenization::Token token : *t.GetTokens()) {
            std::cout << (int)token.type << std::endl;
        }
        
        std::fclose(file);

    }
}
