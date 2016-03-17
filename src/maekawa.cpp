#include "maekawa.h"

Maekawa Maekawa::instance_;

Maekawa& Maekawa::instance(void)
{
    return instance_;
}
