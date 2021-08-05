#include "context.h"
#include <iostream>

using namespace std;

int main()
{
    auto& context = Context::instance();

    // TODO: XStringToKeysym
    //int modifiers = AnyModifier;
    //int modifiers = Mod2Mask;
    //int modifiers = ControlMask | ShiftMask;
    auto divide = XStringToKeysym("KP_Divide");
    auto multiply = XStringToKeysym("KP_Multiply");
    auto subtract = XStringToKeysym("KP_Subtract");
    int modifiers = 0;
    context.set_client(1, divide, modifiers);
    context.set_client(2, multiply, modifiers);
    context.set_client(3, subtract, modifiers);

    context.start();

    context.wait();

    return 0;
}