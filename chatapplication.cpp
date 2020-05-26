#include "chatapplication.h"

ChatApplication* ChatApplication::p_instance = nullptr;

ChatApplication::ChatApplication()
{
}

ChatApplication* ChatApplication::getInstance()
{
    if (p_instance == nullptr) {
        p_instance = new ChatApplication();
    }
    return p_instance;
}
