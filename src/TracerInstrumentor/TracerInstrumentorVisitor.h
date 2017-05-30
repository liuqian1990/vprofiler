#ifndef TRACER_INSTRUMENTOR
#define TRACER_INSTRUMENTOR

// Clang libs
#include "clang/Frontend/FrontendAction.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/Type.h"
#include "clang/AST/Decl.h"
#include "clang/Rewrite/Core/Rewriter.h"

// STL libs
#include <memory>
#include <unordered_map>
#include <string>
#include <vector>
#include <sstream>

#include "FunctionPrototype.h"

class TracerInstrumentorVisitor : public clang::RecursiveASTVisitor<TracerInstrumentorVisitor> {
    private:
        // Context storing additional state
        clang::ASTContext *astContext;

        // Client to rewrite source
        std::shared_ptr<clang::Rewriter> rewriter;

        // Hash map of fully qualified function names to the new function prototypes
        // to be generated by the wrapper generator.  After ParseAST is called on
        // a source file, newPrototypes should have an entry for each function
        // provided by the user which is in the expanded source file (i.e. after 
        // #includes are processed).
        std::unordered_map<std::string, FunctionPrototype> prototypeMap;

        // Name of the target function to instrument and names of its parameters.
        std::vector<std::string> targetFunctionNameAndArgs;

        std::shared_ptr<bool> shouldFlush;

        clang::SourceRange targetFunctionRange;

        const clang::Expr *condExpr;
        clang::SourceRange condRange;
        bool instruDoneForCond;

        int functionIndex;

        std::string functionNameToWrapperName(std::string functionName);

        void fixFunction(const clang::CallExpr *call, const std::string &functionName,
                         bool  isMemberCall);

        void appendNonObjArgs(std::string &newCall, std::vector<const clang::Expr*> &args);

        // Creates the wrapper prototype based on function decl.
        void createNewPrototype(const clang::FunctionDecl *decl,
                                const std::string &functionName,
                                bool isMemberFunc);

        std::string getEntireParamDeclAsString(const clang::ParmVarDecl *decl);

        std::string getContainingFilename(const clang::FunctionDecl *decl);

        bool shouldCreateNewPrototype(const std::string &functionName);

        std::vector<std::string> getFunctionNameAndArgs(const clang::FunctionDecl *decl, bool isMemberFunc);

        bool isTargetFunction(const clang::FunctionDecl *decl, bool isMemberFunc);

        bool inRange(clang::SourceRange largeRange, clang::SourceRange smallRange);

        bool inTargetFunction(const clang::Expr *call);
        bool inTargetFunction(const clang::Stmt *stmt);

        void saveCondExpr(const clang::Expr *cond);

        bool inCondRange(const clang::Expr *call);

    public:
        explicit TracerInstrumentorVisitor(clang::CompilerInstance &ci, 
                              std::shared_ptr<clang::Rewriter> _rewriter,
                              std::string _targetFunctionNameAndArgs,
                              std::shared_ptr<bool> _shouldFlush);

        ~TracerInstrumentorVisitor();

        // Override trigger for when a FunctionDecl is found in the AST
        virtual bool VisitFunctionDecl(const clang::FunctionDecl *decl);

        // Override trigger for when a CXXMethod is found in the AST
        virtual bool VisitCXXMethodDecl(const clang::CXXMethodDecl *decl);

        // Override trigger for when a CallExpr is found in the AST
        virtual bool VisitCallExpr(const clang::CallExpr *call);

        // Override trigger for when a CXXMemberCallExpr is found in the AST
        virtual bool VisitCXXMemberCallExpr(const clang::CXXMemberCallExpr *call);
};

#endif