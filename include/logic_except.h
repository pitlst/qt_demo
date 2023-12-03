#ifndef LOGIC_EXCEPT_H
#define LOGIC_EXCEPT_H

#include <string>
#include <string_view>


namespace DataGroup
{

    class logic_except
    {
    private:
        std::string message;

    public:
        logic_except(std::string_view str = "A problem") : message{str} {}
        std::string_view what() const { return message; }
    };

}
#endif // LOGIC_EXCEPT_H
