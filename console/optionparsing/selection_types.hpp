#include "../../lib/options/user_options.hpp"
#include "../../lib/queue/queues.hpp"

struct sync_options
{
    int retries = 3;
    sync_settings mode;
};

enum class queue_action
{
    add,
    remove,
    clear,
	print
};

struct queue_options
{
    std::vector<std::string> queued;
    queue_action to_do = queue_action::add;
    queues selected;
};