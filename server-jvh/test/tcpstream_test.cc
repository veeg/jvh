#include "test_helper.h"

TEST_F (VideostreamTest, connection)
{
    auto request = from_client.mutable_request ();
    request-set_request_stream_entries (true);

    send_message (request;
}
