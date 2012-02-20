#ifndef CODE_ITERATOR_H_20120118
#define CODE_ITERATOR_H_20120118

#include <string>

namespace quatsch { namespace compiler {

        // Internal use only. This is not necessarily compatible to STL iterators.
        class code_iterator
        {
        public:
                code_iterator (std::string::const_iterator curr)
                : curr_(curr), line_(1), column_(1) {}

                typedef       char  value_type;
                typedef const char* pointer;
                typedef const char& reference;

                reference operator*  () const { return *curr_; }
                pointer   operator-> () const { return &*curr_; }

                code_iterator& operator++ ()
                {
                        if (*curr_ == '\n') next_line(); 
                        else next_column();

                        ++curr_;
                        return *this;
                }
                code_iterator  operator++ (int)
                {
                        code_iterator ci = *this; 
                        ++(*this);
                        return ci;
                }

                bool operator== (code_iterator const &rhs) const {
                        return curr_ == rhs.curr_;
                }

                bool operator!= (code_iterator const &rhs) const {
                        return !(*this == rhs);
                }

                std::string::const_iterator str_iter() const { return curr_; }

        private:
                std::string::const_iterator curr_;
                int line_, column_;

                void next_line() {
                        ++line_;
                        column_ = 1;
                }

                void next_column() {
                        ++column_;
                }
        };

} }

#endif // CODE_ITERATOR_H_20120118
