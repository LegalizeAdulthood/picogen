#include <list>
#include <memory>
#include <string>
#include "phase2/Defun.h" // for Argument

namespace quatsch { namespace compiler { namespace phase3 {

        class Tree;
        typedef std::shared_ptr<Tree> TreePtr;
        typedef std::list<TreePtr>    OperandList;

        class Defun;
        typedef std::shared_ptr<Defun> DefunPtr;
        class Defun {
        public:
                static DefunPtr ForwardDeclare (std::string name,
                                                std::list<phase2::Argument> args,
                                                phase2::Typename returnType);

                void set_body (TreePtr body);

                std::string                 name()      const;
                std::list<phase2::Argument> arguments() const;
                TreePtr                     body()      const;

                phase2::Typename return_type() const;

                size_t arg_count() const;
        private:
                Defun();

        private:
                std::string name_;
                std::list<phase2::Argument> args_;
                TreePtr body_;
                phase2::Typename return_type_;
        };
} } }
