#ifndef MAEKAWA_H
#define MAEKAWA_H

#include "callback_bridge.h"

class Maekawa
{
    public:
        static Maekawa& instance(void);

        inline void subscribe_callbacks(CallbackBridge* cb)
        {
            cb_bridge = cb;
        }

    private:
        Maekawa() { }
        ~Maekawa() { }

        static Maekawa instance_;

        CallbackBridge* cb_bridge;
};

#endif
